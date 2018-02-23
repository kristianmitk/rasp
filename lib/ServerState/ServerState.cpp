#include "ServerState.h"

// TODO: outsource boundaries to a config file
#define MIN_ELECTION_TIMEOUT 300
#define MAX_ELECTION_TIMEOUT 600
#define HEARTBEAT_TIMEOUT 100
#define generateTimeout() random(MIN_ELECTION_TIMEOUT, MAX_ELECTION_TIMEOUT)

#define REQUIRED_VOTES (RASP_NUM_SERVERS / 2 + 1)


followerState_t * ServerState::getFollower(uint32_t id) {
    for (int i = 0; i < NUM_FOLLOWERS; i++) {
        if (followerStates[i].id == id) return &followerStates[i];
    }
}

void ServerState::initialize() {
    receivedVotes   = 0;
    currentTerm     = RASPFS::getInstance().read(RASPFS::CURRENT_TERM);
    votedFor        = RASPFS::getInstance().read(RASPFS::VOTED_FOR);
    commitIndex     = 0;
    lastApplied     = 0;
    log             = new Log();
    role            = FOLLOWER;
    electionTimeout = generateTimeout();
    lastTimeout     = millis();

    int idx = 0;

    for (int i = 0; i < RASP_NUM_SERVERS; i++) {
        if (servers[i].ID != chipID) {
            followerStates[idx].id = servers[i].ID;
            memcpy(followerStates[idx].IP, servers[i].IP, 4);
            idx++;
        }
    }

    this->serialPrint();
}

void ServerState::loopHandler() {
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

        handleAppendEntriesRes(msg);
        msg =  NULL;
        break;

    default:
        Serial.printf("[ERR] received unknown message type: %d", msg->type);
        msg =  NULL;
    }

    if (msg) UDPServer::getInstance().sendPacket(msg->marshall(), msg->size());

    Serial.printf("End at: %lu\n", millis());
}

// TODO: set persistent values
Message * ServerState::handleRequestVoteReq(uint32_t term,
                                            uint32_t candidateID,
                                            uint16_t lastLogIndex,
                                            uint32_t lastLogTerm) {
    rvRes.term        = currentTerm;
    rvRes.voteGranted = false;

    // our state is more up to date than the candidate state
    if (term <= this->currentTerm) {
        Serial.println("VoteGranted = false");

        return &rvRes;
    }

    // if (!this->votedFor || (this->votedFor != chipID) ||
    //     ((this->role == CANDIDATE) && (this->currentTerm < term)))
    if (!this->votedFor || (this->currentTerm < term)) {
        currentTerm = RASPFS::getInstance().write(RASPFS::CURRENT_TERM, term);
        role        = FOLLOWER;
        rvRes.term  = term;

        // SAFETY RULES (§5.2, §5.4)
        if ((lastLogTerm >= log->lastStoredTerm()) &&
            (lastLogIndex >= log->lastIndex()))
        {
            Serial.println("VoteGranted = true");

            votedFor = RASPFS::getInstance().write(RASPFS::VOTED_FOR,
                                                   candidateID);
            rvRes.voteGranted = true;
        }
    } else {
        Serial.printf("Self candidate in term: %d? %d\n",
                      term,
                      ((this->role == CANDIDATE) &&
                       (this->currentTerm == term)));
        Serial.println("VoteGranted = false - 2nd condition");
        return &rvRes;
    }
    resetElectionTimeout();
    return &rvRes;
}

Message * ServerState::handleRequestVoteReq(Message *msg) {
    RequestVoteRequest *p = (RequestVoteRequest *)msg;

    p->serialPrint();
    return handleRequestVoteReq(p->term,
                                p->candidateID,
                                p->lastLogIndex,
                                p->lastLogTerm);
}

void ServerState::handleRequestVoteRes(uint32_t term, uint8_t  voteGranted) {
    Serial.printf("\n\nwithin `handleRequestVoteRes` - ROLE: %d\n", this->role);

    if (this->role == CANDIDATE) {
        if (voteGranted) {
            if (term == this->currentTerm) {
                this->receivedVotes++;
                checkGrantedVotes();
                return;
            } else {
                Serial.printf("Obsolete message: own term:%lu, received: %lu\n",
                              this->currentTerm,
                              term);
            }
        }

        if (this->currentTerm < term) {
            currentTerm = RASPFS::getInstance().write(RASPFS::CURRENT_TERM, term);
            role        = FOLLOWER;
        }
    }
}

void ServerState::handleRequestVoteRes(Message *msg) {
    RequestVoteResponse *p = (RequestVoteResponse *)msg;

    p->serialPrint();
    return handleRequestVoteRes(p->term, p->voteGranted);
}

void ServerState::checkElectionTimeout() {
    if (this->role == LEADER) return;

    rvReq = RequestVoteRequest();

    if (millis() > lastTimeout + electionTimeout) {
        RASPFS::getInstance().write(RASPFS::CURRENT_TERM, (++this->currentTerm));
        printEventHeader(currentTerm);
        Serial.printf(
            "\n[WARN] Election timout. Starting a new election on term: %d\n",
            this->currentTerm);

        role = CANDIDATE;


        // starting a new election always results in first voting for itself
        votedFor      = RASPFS::getInstance().write(RASPFS::VOTED_FOR, chipID);
        receivedVotes = 1;

        rvReq.term         = this->currentTerm;
        rvReq.candidateID  = chipID;
        rvReq.lastLogIndex = this->log->lastIndex();
        rvReq.lastLogTerm  = this->log->lastStoredTerm();

        UDPServer::getInstance().broadcastRequestVoteRPC(rvReq.marshall());
        resetElectionTimeout();
    }
}

void ServerState::checkGrantedVotes() {
    // TODO: better timeouts
    Serial.printf("required: %d, received %d\n", REQUIRED_VOTES, receivedVotes);

    if (REQUIRED_VOTES <= receivedVotes) {
        Serial.printf("ELECTED LEADER!\n", receivedVotes);

        // TODO: set nextIndex/matchIndex for all followes

        for (int i = 0; i < NUM_FOLLOWERS; i++) {
            followerStates[i].matchIndex  = 0;
            followerStates[i].nextIndex   = log->lastIndex() + 1;
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
    aeRes.term     = currentTerm;
    aeRes.success  = 0;
    aeRes.serverId = chipID;

    // (§5.1)
    if (currentTerm > p->term) {
        Serial.printf("Obsolete Message, currentTerm: %lu\n", currentTerm);
        return &aeRes;
    }

    if (p->term > currentTerm) {
        Serial.printf("p->term > currentTerm\n");

        currentTerm = RASPFS::getInstance().write(RASPFS::CURRENT_TERM, p->term);
        aeRes.term  = currentTerm;
    }
    role = FOLLOWER;

    logEntry_t *prevEntry = this->log->getEntry(p->prevLogIndex);


    // check if entry exist at previous log index send from leader
    if (prevEntry) {
        Serial.printf("Entry exist! Size of data: %lu and term:%lu\n",
                      prevEntry->size,
                      prevEntry->term);

        // (§5.3)
        if (prevEntry->term == p->prevLogTerm) {
            Serial.printf("prev Entries do match!\n");

            if (p->dataSize) {
                if (this->log->getTerm(p->prevLogIndex + 1) != p->dataTerm) {
                    // TODO: replace entries that are not commited
                }

                if (!this->log->getEntry(p->prevLogIndex + 1)) {
                    this->log->append(p->dataTerm, p->data, p->dataSize);
                }
            }
            aeRes.success = 1;
        } else {
            Serial.printf("prevEntry.term != p->prevLogTerm\n");
        }
    } else {
        Serial.println("entry does not exist!");

        if (!p->prevLogIndex && !p->prevLogTerm) {
            Serial.println("index=0, term=0");

            if (p->dataSize) {
                if (this->log->getTerm(p->prevLogIndex + 1) != p->dataTerm) {
                    // TODO: replace entries that are not commited
                }

                if (!this->log->getEntry(p->prevLogIndex + 1)) {
                    this->log->append(p->dataTerm, p->data, p->dataSize);
                } else {
                    Serial.println("Entry already appended!");
                }
            }
            aeRes.success = 1;
        }
    }
    free(prevEntry);
    resetElectionTimeout();
    return &aeRes;
}

void ServerState::handleAppendEntriesRes(Message *msg) {
    if (role != LEADER) return;

    AppendEntriesResponse *p = (AppendEntriesResponse *)msg;

    p->serialPrint();

    if (p->term > currentTerm) {
        currentTerm = RASPFS::getInstance().write(RASPFS::CURRENT_TERM, p->term);
        this->role  = FOLLOWER;
        return;
    }
    followerState_t *fstate = getFollower(p->serverId);

    // TODO: minimize code
    if (!p->success) {
        // make sure we never get under 1 - this seemed to happen due to
        // multiple AE send
        fstate->nextIndex = fstate->nextIndex == 1 ? 1 : fstate->nextIndex - 1;
        uint16_t sendIndex = fstate->nextIndex - 1;

        aeReq.term         = currentTerm;
        aeReq.leaderId     = chipID;
        aeReq.leaderCommit = commitIndex;
        aeReq.prevLogIndex = sendIndex;
        aeReq.prevLogTerm  = this->log->getTerm(sendIndex);
        aeReq.dataSize     = 0;
        aeReq.dataTerm     = 0;
        aeReq.data         = NULL;

        logEntry_t *entry = NULL;

        if (!sendIndex && (entry = this->log->getEntry(1))) {
            aeReq.dataSize = entry->size;
            aeReq.data     = (uint8_t *)entry->data;
            aeReq.dataTerm = entry->term;
            free(entry);
        }
        UDPServer::getInstance().sendPacket(aeReq.marshall(), aeReq.size());
        Serial.printf(
            "Send a decremented AE with nextIndex:%d, matchIndex:%d\n",
            fstate->nextIndex,
            fstate->matchIndex);
    } else {
        if (fstate->nextIndex <= this->log->lastIndex()) {
            aeReq.term         = currentTerm;
            aeReq.leaderId     = chipID;
            aeReq.leaderCommit = commitIndex;
            aeReq.prevLogIndex = fstate->nextIndex - 1;
            aeReq.prevLogTerm  = this->log->getTerm(fstate->nextIndex - 1);
            aeReq.dataSize     = 0;
            aeReq.dataTerm     = 0;
            aeReq.data         = NULL;

            logEntry_t *entry = NULL;

            if (entry = this->log->getEntry(fstate->nextIndex)) {
                aeReq.dataSize = entry->size;
                aeReq.data     = (uint8_t *)entry->data;
                aeReq.dataTerm = entry->term;
                free(entry);
            }
            UDPServer::getInstance().sendPacket(aeReq.marshall(), aeReq.size());
            Serial.printf(
                "Send an AE with nextIndex:%d, matchIndex:%d\n",
                fstate->nextIndex,
                fstate->matchIndex
                );
            fstate->nextIndex++;

            // TODO: add matchIndex
            // fstate->matchIndex++;
        }
    }
    fstate->lastTimeout = millis();
}

void ServerState::resetElectionTimeout() {
    electionTimeout = generateTimeout();

    lastTimeout = millis();

    Serial.printf("New timeout: %lu current millis: %lu\n",
                  electionTimeout,
                  lastTimeout);
}

ServerState::Role ServerState::getRole() {
    return this->role;
}

void ServerState::checkHeartbeatTimeouts() {
    if (this->role != LEADER) return;

    for (int i = 0; i < NUM_FOLLOWERS; i++) {
        if (followerStates[i].lastTimeout + heartbeatTimeout < millis()) {
            printEventHeader(currentTerm);
            Serial.printf("lastTimeout: %lu\n", followerStates[i].lastTimeout);

            Serial.printf("Sending AE Req, nextIndex: %lu\n",
                          followerStates[i].nextIndex);

            aeReq.term         = currentTerm;
            aeReq.leaderId     = chipID;
            aeReq.dataSize     = 0;
            aeReq.dataTerm     = 0;
            aeReq.data         = NULL;
            aeReq.prevLogIndex = followerStates[i].nextIndex - 1;
            aeReq.prevLogTerm  = log->getTerm(aeReq.prevLogIndex);

            // if there is anything to append, than do so
            if (followerStates[i].nextIndex <= this->log->lastIndex()) {
                logEntry_t *entry =
                    this->log->getEntry(followerStates[i].nextIndex);
                aeReq.data     = (uint8_t *)entry->data;
                aeReq.dataSize = entry->size;
                aeReq.dataTerm = entry->term;
                Serial.printf("Passing dataTerm:%lu\n", entry->term);
                free(entry);
            }
            UDPServer::getInstance().sendPacket(aeReq.marshall(),
                                                aeReq.size(),
                                                followerStates[i].IP);
            followerStates[i].lastTimeout = millis();
        }
    }
}

// TODO: this might be obsolete - remove
// Message * ServerState::generateEmptyHeartBeat() {
//     aeReq.term         = currentTerm;
//     aeReq.leaderId     = chipID;
//     aeReq.prevLogIndex = this->log->lastIndex();
//     aeReq.prevLogTerm  = this->log->lastStoredTerm();
//     aeReq.leaderCommit = commitIndex;
//     aeReq.dataSize     = 0;
//     return &aeReq;
// }

void ServerState::DEBUG_APPEND_LOG() {
    if (this->role == LEADER) {
        uint32_t rnd = random(1, 1000000);

        if (rnd > 999990) {
            printEventHeader(currentTerm);
            uint16_t entrySize = random(1, 50);
            uint8_t  entryData[entrySize];
            entryData[0] = random(0, 2);

            this->log->append(currentTerm, entryData, entrySize);
        }
    }
}

void ServerState::serialPrint() {
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
