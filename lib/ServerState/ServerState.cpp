#include "ServerState.h"
#include "util.h"

// TODO: outsource boundaries to a config file
#define MIN_ELECTION_TIMEOUT 200
#define MAX_ELECTION_TIMEOUT 500
#define generateTimeout() random(MIN_ELECTION_TIMEOUT, MAX_ELECTION_TIMEOUT)

#define REQUIRED_VOTES (RASP_NUM_SERVERS / 2 + 1)


followerState_t ServerState::getFollower(uint32_t id) {
    for (int i = 0; i < NUM_FOLLOWERS; i++) {
        if (followerStates[i].id == id) return followerStates[i];
    }
}

ServerState::ServerState() {}

void ServerState::initialize() {
    EMPTY_HEARTBEAT = true;

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
            followerStates[idx++].id = servers[i].ID;
        }
    }

    this->serialPrint();
}

Message * ServerState::dispatch(Message *msg) {
    switch (msg->type) {
    case Message::RequestVoteReq:
        return handleRequestVoteReq(msg);

    case Message::RequestVoteRes:
        handleRequestVoteRes(msg);
        return NULL;

    case Message::AppendEntriesReq:
        return handleAppendEntriesReq(msg);

    case Message::AppendEntriesRes:

        handleAppendEntriesRes(msg);
        return NULL;

    default:
#ifdef RASP_DEBUG
        Serial.printf("[ERR] received unknown message type: %d", msg->type);
#endif // ifdef RASP_DEBUG
        return NULL;
    }
    return NULL;
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

#ifdef RASP_DEBUG
            } else {
                Serial.printf("Obsolete message: own term:%lu, received: %lu\n",
                              this->currentTerm,
                              term);
#endif // ifdef RASP_DEBUG
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

Message * ServerState::checkElectionTimeout() {
    if (this->role == LEADER) return NULL;

    rvReq = RequestVoteRequest();

    if (millis() > lastTimeout + electionTimeout) {
        RASPFS::getInstance().write(RASPFS::CURRENT_TERM, (++this->currentTerm));

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

        resetElectionTimeout();
        return &rvReq;
    }
    return NULL;
}

void ServerState::checkGrantedVotes() {
    // TODO: better timeouts
    Serial.printf("required: %d, received %d\n", REQUIRED_VOTES, receivedVotes);

    if (REQUIRED_VOTES <= receivedVotes) {
        Serial.printf("ELECTED LEADER!\n", receivedVotes);

        // TODO: set nextIndex/matchIndex for all followes

        for (int i = 0; i < NUM_FOLLOWERS; i++) {
            followerStates[i].matchIndex = 0;
            followerStates[i].nextIndex  = log->lastIndex() + 1;
        }

        role             = LEADER;
        heartbeatTimeout = 75;
        lastTimeout      = millis();
        Serial.printf("New heartbeatTimeout: %lu\ncurrent millis: %lu\n",
                      heartbeatTimeout,
                      lastTimeout);
    }
}

Message * ServerState::handleAppendEntriesReq(Message *msg) {
    AppendEntriesRequest *p = (AppendEntriesRequest *)msg;

    p->serialPrint();

#ifdef RASP_DEBUG
    Serial.printf("Handling AppendEntries RPC request\n");
#endif // ifdef RASP_DEBUG

    aeRes.term     = currentTerm;
    aeRes.success  = 0;
    aeRes.serverId = chipID;

    // (§5.1)
    if (currentTerm > p->term) {
        return &aeRes;
    }

    if (p->term > currentTerm) {
#ifdef RASP_DEBUG
        Serial.printf("p->term > currentTerm\n");
#endif // ifdef RASP_DEBUG

        currentTerm = RASPFS::getInstance().write(RASPFS::CURRENT_TERM, p->term);
        aeRes.term  = currentTerm;
    }
    role = FOLLOWER;

    logEntry_t prevEntry = this->log->getEntry(p->prevLogIndex);

    // check if entry exist at previous log index send from leader
    if (prevEntry.size) {
#ifdef RASP_DEBUG
        Serial.printf("Entry exist! Size of data: %lu and term:%lu\n",
                      prevEntry.size,
                      prevEntry.term);
#endif // ifdef RASP_DEBUG

        // (§5.3)
        if (prevEntry.term == p->prevLogTerm) {
#ifdef RASP_DEBUG
            Serial.printf("Appending new entry\n");
#endif // ifdef RASP_DEBUG
            this->log->append(p->term, p->data, p->dataSize);

            aeRes.success = 1;
            resetElectionTimeout();
        } else {
#ifdef RASP_DEBUG
            Serial.printf("prevEntry.term != p->prevLogTerm\n");
#endif // ifdef RASP_DEBUG
            return &aeRes;
        }
    }
    return &aeRes;
}

void ServerState::handleAppendEntriesRes(Message *msg) {
    AppendEntriesResponse *p = (AppendEntriesResponse *)msg;

    p->serialPrint();

    if (p->term > currentTerm) {
        currentTerm = RASPFS::getInstance().write(RASPFS::CURRENT_TERM, p->term);
        this->role  = FOLLOWER;
        return;
    }
    followerState_t fstate = getFollower(p->serverId);

    if (!p->success) {
        // should never be the case that we dont have a success and nextIndex <
        // 1
        if (fstate.nextIndex) {
            fstate.nextIndex--;
            Serial.printf(
                "Would send a decremented AE to: %lu\nnextIndex:%d, matchIndex:%d\n",
                fstate.id,
                fstate.nextIndex,
                fstate.matchIndex);

            // TODO: send a decremented append entries message
        }
    } else {
        fstate.nextIndex++;
        fstate.matchIndex++;

        if (fstate.nextIndex <= this->log->lastIndex()) {
            Serial.printf(
                "Would send a incremented AE to: %lu\nnextIndex:%d, matchIndex:%d\n",
                fstate.id,
                fstate.nextIndex,
                fstate.matchIndex
                );

            // TODO: send a incremented append entries message
        }
    }
}

void ServerState::resetElectionTimeout() {
    electionTimeout = generateTimeout();

    lastTimeout = millis();

    Serial.printf("New timeout: %lu\ncurrent millis: %lu\n",
                  electionTimeout,
                  lastTimeout);
}

ServerState::Role ServerState::getRole() {
    return this->role;
}

uint8_t ServerState::checkHeartbeatTimeout() {
    if (this->role != LEADER) return 0;

    uint32_t curr = millis();

    if (curr > lastTimeout + heartbeatTimeout) {
        lastTimeout = curr;
        return 1;
    }
    return 0;
}

Message * ServerState::generateEmptyHeartBeat() {
    aeReq.term         = currentTerm;
    aeReq.leaderId     = chipID;
    aeReq.prevLogIndex = this->log->lastIndex();
    aeReq.prevLogTerm  = this->log->lastStoredTerm();
    aeReq.leaderCommit = commitIndex;
    aeReq.dataSize     = 0;
    return &aeReq;
}

void ServerState::DEBUG_APPEND_LOG() {
    if (this->role == LEADER) {
        uint32_t rnd = random(1, 1000000);

        if (rnd > 999990) {
            uint16_t entrySize = random(1, 50);
            uint8_t  entryData[entrySize];
            entryData[0] = random(0, 2);

            this->log->append(currentTerm, entryData, entrySize);
            this->log->printLastEntry();
            this->EMPTY_HEARTBEAT = false;
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
