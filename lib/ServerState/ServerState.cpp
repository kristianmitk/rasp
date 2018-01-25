#include "ServerState.h"
#include "util.h"

// TODO: outsource boundaries to a config file
#define MIN_ELECTION_TIMEOUT 150
#define MAX_ELECTION_TIMEOUT 300
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

RequestVoteResponse ServerState::handleRequestVoteReq(uint32_t term,
                                                      uint32_t candidateID,
                                                      uint32_t lastLogIndex,
                                                      uint32_t lastLogTerm) {
    RequestVoteResponse res;

    res.term        = currentTerm;
    res.voteGranted = false;

    // our state is more up to date than the candidate state
    if ((term < currentTerm) || (lastLogTerm < log->latestTerm)) {
        Serial.println("VoteGranted = false");
        return res;
    }

    if (!votedFor || candidateID) {
        if ((lastLogTerm > log->latestTerm) ||
            (lastLogIndex >= log->latestIndex)) {
            Serial.println("VoteGranted = true");

            role            = FOLLOWER;
            currentTerm     = term;
            votedFor        = candidateID;
            res.voteGranted = true;
        }
    }

    resetElectionTimeout();

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
    Serial.printf("\n\nROLE: %d", this->role);

    if (this->role == CANDIDATE) {
        // make sure not to count obsolete responses
        if (voteGranted && (term <= this->currentTerm)) {
            this->receivedVotes++;
            checkGrantedVotes();
            return;
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
        msg.lastLogIndex = this->log->latestIndex;
        msg.lastLogTerm  = this->log->latestTerm;

        resetElectionTimeout();
    }
    return msg;
}

void ServerState::checkGrantedVotes() {
    // TODO: better timeouts
    Serial.printf("required: %d, received %d\n", REQUIRED_VOTES, receivedVotes);

    if (REQUIRED_VOTES <= receivedVotes) {
        Serial.printf("ELECTED LEADER!\n", receivedVotes);
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
