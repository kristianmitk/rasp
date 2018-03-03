#include "ServerState.h"
#include "StateMachine.h"
#include "Log.h"

// TODO: outsource boundaries to a config file
// TODO: improve timeouts
#define MIN_ELECTION_TIMEOUT 300
#define MAX_ELECTION_TIMEOUT 600
#define HEARTBEAT_TIMEOUT 100
#define generateTimeout() random(MIN_ELECTION_TIMEOUT, MAX_ELECTION_TIMEOUT)

#define MAJORITY (RASP_NUM_SERVERS / 2 + 1)

#define _LOG Log::getInstance()

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
    currentTerm     = RASPFS::getInstance().readCurrentTerm();
    votedFor        = RASPFS::getInstance().readVotedFor();
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
                  "lastTimeout"
                  );
    Serial.printf("%-25lu%-25lu%-25lu%-25lu\n\n",
                  currentTerm,
                  votedFor,
                  electionTimeout,
                  lastTimeout
                  );
}

void ServerState::loopHandler() {
    if (this->commitIndex > this->lastApplied) {
        printEventHeader(currentTerm);
        this->lastApplied++;
        logEntry_t *entry = _LOG.getEntry(this->lastApplied);
        StateMachine::getInstance().apply(entry->data);
        free(entry);
    }

    DEBUG_APPEND_LOG();

    handleMessage();

    checkHeartbeatTimeouts();

    checkElectionTimeout();
}

void ServerState::handleMessage() {
    Message *msg = UDPServer::getInstance().checkForMessage();

    if (!msg) return;

    switch (msg->type) {
    case Message::RequestVoteReq:
        msg = handleRequestVoteReq(msg);
        break;

    case Message::RequestVoteRes:
        handleRequestVoteRes(msg);
        msg = NULL;
        break;

    case Message::AppendEntriesReq:
        msg =  handleAppendEntriesReq(msg);
        break;

    case Message::AppendEntriesRes:

        // TODO: return message from `handleAppendEntriesRes`
        msg = handleAppendEntriesRes(msg);
        break;

    default:
        Serial.printf("[ERR] received unknown message type: %d", msg->type);
        msg =  NULL;
    }

    if (msg) UDPServer::getInstance().sendPacket(msg->marshall(), msg->size());

    Serial.printf("End at: %lu\n", millis());
}

Message * ServerState::handleRequestVoteReq(Message *msg) {
    RequestVoteRequest *p = (RequestVoteRequest *)msg;

    p->serialPrint();

    rvRes.term        = currentTerm;
    rvRes.voteGranted = false;

    // our state is more up to date than the candidate state
    if (p->term <= this->currentTerm) {
        Serial.println("VoteGranted = false");

        return &rvRes;
    }

    if (!this->votedFor || (this->currentTerm < p->term)) {
        currentTerm = RASPFS::getInstance().writeCurrentTerm(p->term);
        role        = FOLLOWER;
        rvRes.term  = p->term;

        // SAFETY RULES (§5.2, §5.4)
        if ((p->lastLogTerm > _LOG.lastStoredTerm()) ||
            ((p->lastLogTerm == _LOG.lastStoredTerm()) &&
             (p->lastLogIndex >= _LOG.lastIndex())))
        {
            Serial.println("VoteGranted = true");

            votedFor =
                RASPFS::getInstance().writeVotedFor(p->candidateID);
            rvRes.voteGranted = true;
        } else {
            return &rvRes;
        }
    } else {
        Serial.printf("Self candidate in term: %d? %d\n",
                      p->term,
                      ((this->role == CANDIDATE) &&
                       (this->currentTerm == p->term)));
        Serial.println("VoteGranted = false - 2nd condition");
        return &rvRes;
    }
    resetElectionTimeout();
    return &rvRes;
}

void ServerState::handleRequestVoteRes(Message *msg) {
    RequestVoteResponse *p = (RequestVoteResponse *)msg;

    p->serialPrint();
    Serial.printf("\n\nwithin `handleRequestVoteRes` - ROLE: %d\n", this->role);

    if (this->role == CANDIDATE) {
        if (p->voteGranted) {
            if (p->term == this->currentTerm) {
                this->receivedVotes++;
                checkGrantedVotes();
                return;
            } else {
                Serial.printf("Obsolete message: own term:%lu, received: %lu\n",
                              this->currentTerm,
                              p->term);
            }
        }

        if (this->currentTerm < p->term) {
            currentTerm = RASPFS::getInstance().writeCurrentTerm(p->term);
            role        = FOLLOWER;
        }
    }
}

void ServerState::checkElectionTimeout() {
    if (this->role == LEADER) return;

    // empty message from previous usage
    rvReq = RequestVoteRequest();

    if (millis() > lastTimeout + electionTimeout) {
        RASPFS::getInstance().writeCurrentTerm(++this->currentTerm);
        printEventHeader(currentTerm);
        Serial.printf(
            "\n[WARN] Election timout. Starting a new election on term: %d\n",
            this->currentTerm);

        role = CANDIDATE;

        // starting a new election always results in first voting for itself
        votedFor      = RASPFS::getInstance().writeVotedFor(chipId);
        receivedVotes = 1;

        rvReq.term         = this->currentTerm;
        rvReq.candidateID  = chipId;
        rvReq.lastLogIndex = _LOG.lastIndex();
        rvReq.lastLogTerm  = _LOG.lastStoredTerm();

        UDPServer::getInstance().broadcastRequestVoteRPC(rvReq.marshall());
        resetElectionTimeout();
    }
}

void ServerState::checkGrantedVotes() {
    // TODO: better timeouts
    Serial.printf("required: %d, received %d\n", MAJORITY, receivedVotes);

    if (MAJORITY <= receivedVotes) {
        Serial.printf("ELECTED LEADER!\n", receivedVotes);

        for (int i = 0; i < NUM_FOLLOWERS; i++) {
            followerStates[i].matchIndex  = 0;
            followerStates[i].nextIndex   = _LOG.lastIndex() + 1;
            followerStates[i].lastTimeout = millis();
        }

        role             = LEADER;
        heartbeatTimeout = HEARTBEAT_TIMEOUT;
        Serial.printf("New heartbeatTimeout: %lu\ncurrent millis: %lu\n",
                      heartbeatTimeout,
                      lastTimeout);
    }
}

Message * ServerState::handleAppendEntriesReq(Message *msg) {
    AppendEntriesRequest *p = (AppendEntriesRequest *)msg;

    p->serialPrint();

    Serial.printf("Handling AppendEntries RPC request\n");
    aeRes.term       = currentTerm;
    aeRes.success    = 0;
    aeRes.matchIndex = 0;
    aeRes.serverId   = chipId;

    // (§5.1)
    if (currentTerm > p->term) {
        Serial.printf("Obsolete Message, currentTerm: %lu\n", currentTerm);
        return &aeRes;
    }

    if (p->term > currentTerm) {
        Serial.printf("p->term > currentTerm\n");
        this->currentTerm = RASPFS::getInstance().writeCurrentTerm(p->term);
        aeRes.term        = currentTerm;
    }

    if (p->leaderCommit > this->commitIndex) {
        this->commitIndex = min(p->leaderCommit, _LOG.lastIndex());
    }

    role = FOLLOWER;

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

        if (p->dataSize) {
            if (_LOG.exist(p->prevLogIndex + 1) &&
                (_LOG.getTerm(p->prevLogIndex + 1) != p->dataTerm)) {
                _LOG.truncate(p->prevLogIndex);
            }

            if (!_LOG.getEntry(p->prevLogIndex + 1)) {
                _LOG.append(p->dataTerm, p->data, p->dataSize);
                aeRes.matchIndex++;
            }
        }
    }

    free(prevEntry);
    resetElectionTimeout();
    return &aeRes;
}

Message * ServerState::handleAppendEntriesRes(Message *msg) {
    if (role != LEADER) return NULL;

    AppendEntriesResponse *p = (AppendEntriesResponse *)msg;

    p->serialPrint();

    if (p->term > currentTerm) {
        currentTerm = RASPFS::getInstance().writeCurrentTerm(p->term);
        this->role  = FOLLOWER;
        return NULL;
    }

    followerState_t *fstate = getFollower(p->serverId);

    // in case of a success matchIndex can only be incremented
    // but if its just a reply to an empty heartbeat we dont have to check
    // for new commit index...
    if (fstate->matchIndex < p->matchIndex) {
        Serial.printf("new machIndex is higher than: %lu\n", fstate->matchIndex);
        fstate->matchIndex = p->matchIndex;
        checkForNewCommitedIndex();
    }

    // adjust next index to send, givne the matching information from other peer
    fstate->nextIndex = max(fstate->nextIndex, uint16_t(p->matchIndex + 1));

    // decrement the nextIndex if we did not received a success
    if (!p->success) {
        // make sure we never get under 1 - this may happen due to multiple
        // AE arriving
        fstate->nextIndex = max((fstate->nextIndex - 1), 1);

        // fstate->nextIndex -= fstate->nextIndex == 1 ? 0 : 1;

        Serial.printf(
            "Sending a decremented AE with nextIndex:%d, matchIndex:%d\n",
            fstate->nextIndex,
            fstate->matchIndex);
    } else {
        // we have nothing to send if we received a success and nextIndex is
        // highter than the last index in the log    (- can be max. highter by
        // 1)
        if (fstate->nextIndex > _LOG.lastIndex()) return NULL;
    }

    uint16_t prevIndex = fstate->nextIndex - 1;
    createAERequestMessage(fstate, p->success);

    fstate->lastTimeout = millis();
    return &aeReq;
}

void ServerState::resetElectionTimeout() {
    electionTimeout = generateTimeout();

    lastTimeout = millis();

    Serial.printf("New timeout: %lu current millis: %lu\n",
                  electionTimeout,
                  lastTimeout);
}

void ServerState::checkHeartbeatTimeouts() {
    if (this->role != LEADER) return;

    for (int i = 0; i < NUM_FOLLOWERS; i++) {
        if (followerStates[i].lastTimeout + heartbeatTimeout < millis()) {
            printEventHeader(currentTerm);
            Serial.printf("lastTimeout: %lu\n", followerStates[i].lastTimeout);

            Serial.printf("Sending AE Req, nextIndex: %lu\n",
                          followerStates[i].nextIndex);

            createAERequestMessage(&followerStates[i], true);

            UDPServer::getInstance().sendPacket(aeReq.marshall(),
                                                aeReq.size(),
                                                followerStates[i].IP);
            followerStates[i].lastTimeout = millis();
        }
    }
}

void ServerState::createAERequestMessage(followerState_t *fstate,
                                         bool             success) {
    aeReq.term         = currentTerm;
    aeReq.leaderId     = chipId;
    aeReq.leaderCommit = commitIndex;
    aeReq.dataSize     = 0;
    aeReq.dataTerm     = 0;
    aeReq.data         = NULL;
    uint16_t prevIndex = fstate->nextIndex - 1;
    aeReq.prevLogIndex = prevIndex;
    aeReq.prevLogTerm  = _LOG.getTerm(prevIndex);

    logEntry_t *entry = _LOG.getEntry(fstate->nextIndex);

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
        free(entry);
    }
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

// TODO: remove console prints
void ServerState::checkForNewCommitedIndex() {
    if (this->role != LEADER) return;

    Serial.println("Checking new Index");
    uint16_t matchIndexes[NUM_FOLLOWERS];

    for (int i = 0; i < NUM_FOLLOWERS; i++) {
        matchIndexes[i] = followerStates[i].matchIndex;
        Serial.printf("%d: %lu\n", i, matchIndexes[i]);
    }

    Serial.println("Bout to start sorting");
    qsort(matchIndexes, NUM_FOLLOWERS, sizeof(matchIndexes[0]), compare);
    Serial.println("After sort:");

    for (int i = 0; i < NUM_FOLLOWERS; i++) {
        Serial.printf("%d: %lu\n", i, matchIndexes[i]);
    }
    Serial.printf("Returning number at index: %d\n",
                  NUM_FOLLOWERS - (MAJORITY - 1));

    // get the highes index that is replicated on the majority
    uint16_t tempCommit = matchIndexes[NUM_FOLLOWERS - (MAJORITY - 1)];
    Serial.printf("Temp commit is: %d\n", tempCommit);

    // only assign to servers commitIndex if log entry at potential commit index
    // has the same term as the current leaders term
    // term
    if ((tempCommit > commitIndex) &&
        (_LOG.getTerm(tempCommit) == this->currentTerm)) {
        this->commitIndex = tempCommit;
    }
}

void ServerState::DEBUG_APPEND_LOG() {
    if (this->role == LEADER) {
        uint32_t rnd = random(1, 1000000);

        if (rnd > 999990) {
            printEventHeader(currentTerm);
            uint16_t entrySize = random(1, 50);
            uint8_t  entryData[entrySize];
            entryData[0] = random(0, 2);

            _LOG.append(currentTerm, entryData, entrySize);
        }
    }
}
