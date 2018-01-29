#include "ServerState.h"
#include "util.h"

// TODO: outsource boundaries to a config file
#define MIN_ELECTION_TIMEOUT 250
#define MAX_ELECTION_TIMEOUT 600
#define generateTimeout() random(MIN_ELECTION_TIMEOUT, MAX_ELECTION_TIMEOUT)

#define REQUIRED_VOTES (RASP_NUM_SERVERS / 2 + 1)

ServerState::ServerState(uint32_t id) {
    selfID          = id;
    receivedVotes   = 0;
    currentTerm     = RASPFS::getInstance().read(CURRENT_TERM);
    votedFor        = RASPFS::getInstance().read(VOTED_FOR);
    commitIndex     = 0;
    lastApplied     = 0;
    log             = new Log();
    role            = FOLLOWER;
    electionTimeout = generateTimeout();
    lastTimeout     = millis();

    this->serialPrint();
}

// TODO: set persistent values
RequestVoteResponse ServerState::handleRequestVoteReq(uint32_t term,
                                                      uint32_t candidateID,
                                                      uint32_t lastLogIndex,
                                                      uint32_t lastLogTerm) {
    RequestVoteResponse res;

    currentTerm     = max(currentTerm, term);
    res.term        = currentTerm;
    res.voteGranted = false;

    // our state is more up to date than the candidate state
    if ((term < currentTerm) || (lastLogTerm < log->lastStoredTerm())) {
        Serial.println("VoteGranted = false");
        return res;
    }

    if (!this->votedFor ||
        (this->votedFor != selfID) ||
        ((this->votedFor == selfID) && (this->role != CANDIDATE))) {
        if ((lastLogTerm > log->lastStoredTerm()) ||
            (lastLogIndex >= log->size())) {
            Serial.println("VoteGranted = true");

            role            = FOLLOWER;
            votedFor        = candidateID;
            res.term        = term;
            res.voteGranted = true;

            resetElectionTimeout();
        }
    } else {
        Serial.printf("Self candidate? %d\n",
                      ((this->votedFor == selfID) && (this->role == CANDIDATE)));
        Serial.println("VoteGranted = false - 2nd condition");
    }

    return res;
}

RequestVoteResponse ServerState::handleRequestVoteReq(RequestVoteRequest msg) {
    msg.serialPrint();
    return handleRequestVoteReq(msg.term,
                                msg.candidateID,
                                msg.lastLogIndex,
                                msg.lastLogTerm);
}

void ServerState::handleRequestVoteRes(uint32_t term, uint8_t  voteGranted) {
    Serial.printf("\n\nwithin `handleRequestVoteRes` - ROLE: %d\n", this->role);

    if (this->role == CANDIDATE) {
        // make sure not to count obsolete responses
        if (voteGranted) {
            if (term == this->currentTerm) {
                this->receivedVotes++;
                checkGrantedVotes();
                return;
            } else {
                // TODO: remove debug logging
                Serial.printf("TERMS; own:%lu, received: %lu\n",
                              this->currentTerm,
                              term);
            }
        }

        if (this->currentTerm < term) {
            this->currentTerm = term;
            role              = FOLLOWER;
        }
    }
}

void ServerState::handleRequestVoteRes(RequestVoteResponse msg) {
    msg.serialPrint();
    return handleRequestVoteRes(msg.term,
                                msg.voteGranted);
}

RequestVoteRequest ServerState::checkElectionTimeout() {
    RequestVoteRequest msg;

    if (this->role == LEADER) return msg;

    if (millis() > lastTimeout + electionTimeout) {
        Serial.printf("\n[WARN] Election timout. Starting a new election\n");
        role = CANDIDATE;

        // Serial.printf("\nBEFORE:%lu\n", millis());

        RASPFS::getInstance().write(CURRENT_TERM, ++this->currentTerm);

        // Serial.printf("\nAFTER:%lu\n", millis());

        // starting a new election always results in first voting for itself
        receivedVotes    = 1;
        votedFor         = RASPFS::getInstance().write(VOTED_FOR, selfID);
        msg.term         = this->currentTerm;
        msg.candidateID  = this->selfID;
        msg.lastLogIndex = this->log->size();
        msg.lastLogTerm  = this->log->lastStoredTerm();

        resetElectionTimeout();
    }
    return msg;
}

void ServerState::checkGrantedVotes() {
    // TODO: better timeouts
    Serial.printf("required: %d, received %d\n", REQUIRED_VOTES, receivedVotes);

    if (REQUIRED_VOTES <= receivedVotes) {
        Serial.printf("ELECTED LEADER!\n", receivedVotes);

        // TODO: set nextIndex/matchIndex for all followes
        role             = LEADER;
        heartbeatTimeout = random(100, 150);
        lastTimeout      = millis();
        Serial.printf("New heartbeatTimeout: %lu\ncurrent millis: %lu\n",
                      heartbeatTimeout,
                      lastTimeout);
    }
}

void ServerState::resetElectionTimeout() {
    electionTimeout = generateTimeout();
    lastTimeout     = millis();

    Serial.printf("New timeout: %lu\ncurrent millis: %lu\n",
                  electionTimeout,
                  lastTimeout);
}

void ServerState::resetElectionTimeout(uint8_t placeholder) {
    role = FOLLOWER;
    resetElectionTimeout();
}

Role ServerState::getRole() {
    return this->role;
}

uint8_t ServerState::checkHeartbeatTimeout() {
    if (this->role != LEADER) return 0;

    if (millis() > lastTimeout + heartbeatTimeout) {
        lastTimeout = millis();
        return 1;
    }
    return 0;
}

void ServerState::DEBUG_APPEND_LOG() {
    if (this->role == LEADER) {
        uint32_t rnd = random(1, 1000000);

        if (rnd > 999990) {
            Serial.printf("\n%luy",        rnd);
            Serial.printf("Before: %lu\n", millis());
            logEntry_t newEntry;
            newEntry.term = this->currentTerm;

            if (random(1, 100) > 50) {
                newEntry.data[0] = 1;
            } else {
                newEntry.data[0] = 0;
            }
            this->log->append(newEntry);
            this->log->printLastEntry();
            Serial.printf("After: %lu\n", millis());
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
    Serial.printf("%-25lu%-25lu%-25lu%-25lu\n",
                  currentTerm,
                  votedFor,
                  electionTimeout,
                  lastTimeout
                  );
}
