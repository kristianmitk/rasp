#include "ServerState.h"
#include "Log.h"
#include "config.h"
#include "StateMachine.h"

#define generateTimeout() random(MIN_ELECTION_TIMEOUT, MAX_ELECTION_TIMEOUT)
#define MAJORITY (RASP_NUM_SERVERS / 2 + 1)

#define _LOG        Log::getInstance()
#define _UDPServer  UDPServer::getInstance()
#define _RASPFS     RASPFS::getInstance()

StateMachine sm;

followerState_t * ServerState::getFollower(uint32_t id) {
    for (int i = 0; i < NUM_FOLLOWERS; i++) {
        if (followerStates[i].id == id) return &followerStates[i];
    }
}

void ServerState::initialize() {
    leaderId        = 0;
    receivedVotes   = 0;
    commitIndex     = 0;
    lastApplied     = 0;
    role            = FOLLOWER;
    currentTerm     = _RASPFS.readCurrentTerm();
    votedFor        = _RASPFS.readVotedFor();
    electionTimeout = generateTimeout();
    lastTimeout     = millis();
    _LOG.initialize();

    int idx = 0;

    for (int i = 0; i < RASP_NUM_SERVERS; i++) {
        if (servers[i].ID != chipId) {
            followerStates[idx].id = servers[i].ID;
            memcpy(followerStates[idx].IP, servers[i].IP, 4);
            idx++;
        }
    }

    Serial.printf("Server state:\n%-25s%-25s%-25s%-25s\n",
                  "currentTerm",
                  "votedFor",
                  "electionTimeout",
                  "lastLogIndex"
                  );
    Serial.printf("%-25lu%-25lu%-25lu%-25lu\n\n",
                  currentTerm,
                  votedFor,
                  electionTimeout,
                  _LOG.lastIndex()
                  );
}

void ServerState::loopHandler() {
    handleMessage();

    checkHeartbeatTimeouts();

    checkElectionTimeout();

    checkForNewSMcommands();
}

void ServerState::handleMessage() {
    Message *msg = _UDPServer.checkForMessage();

    if (!msg) return;

    switch (msg->type) {
    case Message::RequestVoteReq:
        msg = handleRequestVoteReq(msg);
        break;

    case Message::RequestVoteRes:
        msg = handleRequestVoteRes(msg);
        break;

    case Message::AppendEntriesReq:
        msg = handleAppendEntriesReq(msg);
        break;

    case Message::AppendEntriesRes:
        msg = handleAppendEntriesRes(msg);
        break;

    case Message::StateMachineReadReq:
        msg = handleSMreadReq(msg);
        break;

    case Message::StateMachineWriteReq:
        msg = handleSMwriteReq(msg);
        break;

    default:
        RASPDBG("[ERR] ServerState received unknown message type: %d", msg->type)
        msg =  NULL;
    }

    if (msg) _UDPServer.sendPacket(msg->marshall(), msg->size());

    RASPDBG("End at: %lu\n", millis())
}

Message * ServerState::handleRequestVoteReq(Message *msg) {
    RequestVoteRequest *p = (RequestVoteRequest *)msg;

    p->serialPrint();

    rvRes.term        = currentTerm;
    rvRes.voteGranted = false;

    // our state is more up to date than the candidate state
    if (p->term <= this->currentTerm) {
        RASPDBG("VoteGranted = false\n")
        return &rvRes;
    }

    if (!this->votedFor || (this->currentTerm < p->term)) {
        currentTerm = _RASPFS.writeCurrentTerm(p->term);
        role        = FOLLOWER;
        rvRes.term  = p->term;

        // SAFETY RULES (§5.2, §5.4)
        if ((p->lastLogTerm > _LOG.lastStoredTerm()) ||
            ((p->lastLogTerm == _LOG.lastStoredTerm()) &&
             (p->lastLogIndex >= _LOG.lastIndex()))) {
            RASPDBG("VoteGranted = true\n")

            votedFor          = _RASPFS.writeVotedFor(p->candidateID);
            rvRes.voteGranted = true;
        } else {
            return &rvRes;
        }
    } else {
        RASPDBG("Self candidate in term: %d? %d\n",
                p->term,
                ((this->role == CANDIDATE) &&
                 (this->currentTerm == p->term)))
        RASPDBG("VoteGranted = false - 2nd condition")
        return &rvRes;
    }
    resetElectionTimeout();
    return &rvRes;
}

Message * ServerState::handleRequestVoteRes(Message *msg) {
    RequestVoteResponse *p = (RequestVoteResponse *)msg;

    p->serialPrint();
    RASPDBG("\n\nwithin `handleRequestVoteRes` - ROLE: %d\n", this->role)

    if (this->role == CANDIDATE) {
        if (p->voteGranted) {
            if (p->term == this->currentTerm) {
                this->receivedVotes++;
                checkGrantedVotes();
                return NULL;
            } else {
                RASPDBG("Obsolete message: own term:%lu, received: %lu\n",
                        this->currentTerm,
                        p->term)
            }
        }

        if (this->currentTerm < p->term) {
            currentTerm = _RASPFS.writeCurrentTerm(p->term);
            role        = FOLLOWER;
        }
    }
    return NULL;
}

void ServerState::checkElectionTimeout() {
    if (this->role == LEADER) return;

    // empty message from previous usage
    rvReq = RequestVoteRequest();

    if (millis() > lastTimeout + electionTimeout) {
        _RASPFS.writeCurrentTerm(++this->currentTerm);
        printEventHeader(currentTerm);
        RASPDBG("\n[WARN] Election timout. Starting a new election on term: %d\n",
                this->currentTerm)

        role = CANDIDATE;

        // starting a new election always results in first voting for itself
        votedFor      = _RASPFS.writeVotedFor(chipId);
        receivedVotes = 1;

        rvReq.term         = this->currentTerm;
        rvReq.candidateID  = chipId;
        rvReq.lastLogIndex = _LOG.lastIndex();
        rvReq.lastLogTerm  = _LOG.lastStoredTerm();

        _UDPServer.broadcastRequestVoteRPC(rvReq.marshall());
        resetElectionTimeout();
    }
}

void ServerState::checkGrantedVotes() {
    // TODO: better timeouts
    RASPDBG("required: %d, received %d\n", MAJORITY, receivedVotes)

    if (MAJORITY <= receivedVotes) {
        Serial.printf("ELECTED LEADER!");

        for (int i = 0; i < NUM_FOLLOWERS; i++) {
            followerStates[i].matchIndex  = 0;
            followerStates[i].nextIndex   = _LOG.lastIndex() + 1;
            followerStates[i].lastTimeout = millis();
        }

        role             = LEADER;
        heartbeatTimeout = HEARTBEAT_TIMEOUT;
        RASPDBG("New heartbeatTimeout: %lu\ncurrent millis: %lu\n",
                heartbeatTimeout,
                lastTimeout)
    }
}

Message * ServerState::handleAppendEntriesReq(Message *msg) {
    AppendEntriesRequest *p = (AppendEntriesRequest *)msg;

    p->serialPrint();

    RASPDBG("Handling AppendEntries RPC request\n")
    aeRes.term       = currentTerm;
    aeRes.success    = 0;
    aeRes.matchIndex = 0;
    aeRes.serverId   = chipId;

    // (§5.1)
    if (currentTerm > p->term) {
        RASPDBG("Obsolete Message, currentTerm: %lu\n", currentTerm)
        return &aeRes;
    }

    if (p->term > currentTerm) {
        RASPDBG("p->term > currentTerm\n")
        this->currentTerm = _RASPFS.writeCurrentTerm(p->term);
        aeRes.term        = currentTerm;
    }

    role     = FOLLOWER;
    leaderId = p->leaderId;

    logEntry_t *prevEntry = _LOG.getEntry(p->prevLogIndex);

    // Case 1:
    //      Previous entry exist and the term numbers do match with the leader
    // Case 2:
    //      Previous entry does not exist, also not at the leader log
    //          -> potential appending of first entry, if message carries data
    if ((prevEntry && (prevEntry->term == p->prevLogTerm)) ||
        (!prevEntry && !p->prevLogIndex && !p->prevLogTerm)) {
        aeRes.success    = 1;
        aeRes.matchIndex = p->prevLogIndex;
        RASPDBG("SHOULD APPEND\n")

        if (p->dataSize) {
            RASPDBG("DATA SIZE EXIST!\n")
            uint16_t nextIndex = p->prevLogIndex + 1;

            if (_LOG.exist(nextIndex)) {
                if (_LOG.getTerm(nextIndex) != p->dataTerm) {
                    RASPDBG("TRUNCATING!\n")
                    _LOG.truncate(p->prevLogIndex);
                } else {
                    // we need to check on this as we would send a wrong
                    // matchIndex value back and withing the next condition
                    // never append a new entry as its already existing, so the
                    // condition would never be satisfied
                    //
                    // also its safe to do that due to the leader-append-only
                    // safety roule - if index and term do match then the entry
                    // is the same
                    RASPDBG("ALREADY EXISTING\n")
                    aeRes.matchIndex++;
                }
            }

            // we might removed the entry in previous if block
            if (!_LOG.exist(nextIndex)) {
                RASPDBG("APPENDING!\n")
                _LOG.append(p->dataTerm, p->data, p->dataSize);
                aeRes.matchIndex++;
            }
        }
    }

    free(prevEntry);

    if (p->leaderCommit > this->commitIndex) {
        this->commitIndex = min(p->leaderCommit, _LOG.lastIndex());
    }

    resetElectionTimeout();
    return &aeRes;
}

Message * ServerState::handleAppendEntriesRes(Message *msg) {
    if (role != LEADER) return NULL;

    AppendEntriesResponse *p = (AppendEntriesResponse *)msg;

    p->serialPrint();

    if (p->term > currentTerm) {
        currentTerm = _RASPFS.writeCurrentTerm(p->term);
        this->role  = FOLLOWER;
        return NULL;
    }

    followerState_t *fstate = getFollower(p->serverId);

    // in case of a success matchIndex can only be incremented
    // but if its just a reply to an empty heartbeat we dont have to check
    // for new commit index...
    if (fstate->matchIndex < p->matchIndex) {
        fstate->matchIndex = p->matchIndex;

        if (fstate->matchIndex > commitIndex) checkForNewCommitedIndex();
    }

    // adjust next index to send, givne the matching information from other peer
    fstate->nextIndex = max(fstate->nextIndex, uint16_t(p->matchIndex + 1));

    // decrement the nextIndex if we did not received a success
    if (!p->success) {
        // make sure we never get under 1 - this may happen due to multiple
        // AE arriving
        fstate->nextIndex = max((fstate->nextIndex - 1), 1);

        // fstate->nextIndex -= fstate->nextIndex == 1 ? 0 : 1;

        RASPDBG(
            "Sending a decremented AE with nextIndex:%d, matchIndex:%d\n",
            fstate->nextIndex,
            fstate->matchIndex)
    } else {
        // we have nothing to send if we received a success and nextIndex is
        // highter than the last index in the log    (- can be max. highter by
        // 1)
        if (fstate->nextIndex > _LOG.lastIndex()) return NULL;
    }
    createAERequestMessage(fstate, p->success);
    aeReq.serialPrint();
    fstate->lastTimeout = millis();

    return &aeReq;
}

void ServerState::resetElectionTimeout() {
    electionTimeout = generateTimeout();

    lastTimeout = millis();

    RASPDBG("New timeout: %lu current millis: %lu\n",
            electionTimeout,
            lastTimeout)
}

void ServerState::checkHeartbeatTimeouts() {
    if (this->role != LEADER) return;

    for (int i = 0; i < NUM_FOLLOWERS; i++) {
        if (followerStates[i].lastTimeout + heartbeatTimeout < millis()) {
            printEventHeader(currentTerm);
            RASPDBG("lastTimeout: %lu\n",
                    followerStates[i].lastTimeout)

            RASPDBG("Sending AE Req, nextIndex: %lu\n",
                    followerStates[i].nextIndex)

            createAERequestMessage(&followerStates[i], true);

            _UDPServer.sendPacket(aeReq.marshall(),
                                  aeReq.size(),
                                  followerStates[i].IP);
            followerStates[i].lastTimeout = millis();
        }
    }
}

void ServerState::createAERequestMessage(followerState_t *fstate,
                                         bool             success) {
    uint16_t prevIndex = fstate->nextIndex - 1;
    logEntry_t *entry  = _LOG.getEntry(fstate->nextIndex);

    aeReq.term         = currentTerm;
    aeReq.leaderId     = chipId;
    aeReq.leaderCommit = commitIndex;
    aeReq.dataSize     = 0;
    aeReq.dataTerm     = 0;
    aeReq.data         = NULL;
    aeReq.prevLogIndex = prevIndex;
    aeReq.prevLogTerm  = _LOG.getTerm(prevIndex);

    // Case 1:
    //      Either we had success and there is still data do be send
    // Case 2:
    //      We had no success and the nextIndex would be the first entry in the
    //      log - for that one we dont have to check if previous entries do
    //      match. In order to avoid sending this message in the next round, we
    //      do it right now and save one roundtrip
    if ((success && entry) || (!success && !prevIndex && entry)) {
        aeReq.data     = (uint8_t *)entry->data;
        aeReq.dataSize = entry->size;
        aeReq.dataTerm = entry->term;
    }

    if (entry) free(entry);
}

/**
 * TODO: DOCS
 * [compare description]
 * @param  e1 [description]
 * @param  e2 [description]
 * @return    [description]
 */
int compare(const void *e1, const void *e2) {
    uint16_t f = *((uint16_t *)e1);
    uint16_t s = *((uint16_t *)e2);

    return (f > s) - (f < s);
}

// TODO: can we just simply iterate over the array instead of sorting?
// value changes max by 1...
void ServerState::checkForNewCommitedIndex() {
    if (this->role != LEADER) return;

    uint16_t matchIndexes[NUM_FOLLOWERS];

    for (int i = 0; i < NUM_FOLLOWERS; i++) {
        matchIndexes[i] = followerStates[i].matchIndex;
    }

    qsort(matchIndexes, NUM_FOLLOWERS, sizeof(matchIndexes[0]), compare);

    // get the highes index that is replicated on the majority
    uint16_t tempCommit = matchIndexes[NUM_FOLLOWERS - (MAJORITY - 1)];
    RASPDBG("Temp commit is: %d\n", tempCommit);

    // only assign to servers commitIndex if log entry at potential commit index
    // has the same term as the current leaders term
    // term
    if ((tempCommit > commitIndex) &&
        (_LOG.getTerm(tempCommit) == this->currentTerm)) {
        this->commitIndex = tempCommit;
    }
}

uint16_t ServerState::append(uint8_t *data, uint16_t size) {
    if (this->role == LEADER) {
        return _LOG.append(currentTerm, data, size);
    }
    return 0;
}

Message * ServerState::handleSMreadReq(Message *msg) {
    // TODO: more sophisticated would be to return data after majority of
    // followers responded to heartbeat message, to make sure leader is not
    // obsolete
    StateMachineMessage *p = (StateMachineMessage *)msg;

    p->serialPrint();

    if (this->role != LEADER) return clientRedirectMessage();

    smData_t *smRes = sm.read(p->data, p->dataSize);
    smMsg.type     = Message::StateMachineReadRes;
    smMsg.data     = smRes->data;
    smMsg.dataSize = smRes->size;
    return &smMsg;
}

Message * ServerState::handleSMwriteReq(Message *msg) {
    StateMachineMessage *p = (StateMachineMessage *)msg;

    p->serialPrint();

    if (this->role != LEADER) return clientRedirectMessage();

    _UDPServer.createClientRequest(this->append(p->data, p->dataSize));
    return NULL;
}

Message * ServerState::clientRedirectMessage() {
    smMsg.type = Message::followerRedirect;

    uint8_t *buf = new uint8_t[4];

    memcpy(buf, getFollower(leaderId)->IP, 4);

    smMsg.data     = buf;
    smMsg.dataSize = 4;
    return &smMsg;
}

void ServerState::checkForNewSMcommands() {
    if (this->commitIndex > this->lastApplied) {
        printEventHeader(currentTerm);
        this->lastApplied++;
        logEntry_t *entry = _LOG.getEntry(this->lastApplied);

        if ((this->role == LEADER) && (entry->term == this->currentTerm)) {
            smData_t *smRes = sm.apply(
                entry->data,
                entry->size);
            std::vector<clientRequest_t>::iterator it =
                std::find_if(
                    _UDPServer.requests.begin(),
                    _UDPServer.requests.end(),
                    findClient(this->lastApplied));
            smMsg.type     = Message::StateMachineWriteRes;
            smMsg.data     = smRes->data;
            smMsg.dataSize = smRes->size;
            _UDPServer.sendPacket(smMsg.marshall(),
                                  smMsg.size(),
                                  it->ip,
                                  it->port);
            RASPDBG("size before erase: %d\n", _UDPServer.requests.size())
            _UDPServer.requests.erase(it);
            RASPDBG("size after erase: %d\n",  _UDPServer.requests.size())
        } else {
            sm.apply(entry->data, entry->size);
        }
        free(entry);
    }
}
