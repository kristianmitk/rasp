#include "ServerState.h"


// TODO: outsource boundaries to a config file
#define MIN_ELECTION_TIMEOUT 150
#define MAX_ELECTION_TIMEOUT 300
#define generateTimeout() random(MIN_ELECTION_TIMEOUT, MAX_ELECTION_TIMEOUT)

// majority of the cluster
#define requiredVotes (RASP_NUM_SERVERS / 2 + 1)

ServerState::ServerState(uint32_t id) {
    receivedVotes   = 0;
    selfID          = id;
    currentTerm     = 0;
    votedFor        = 0;
    log             = new Log();
    commitIndex     = 0;
    lastApplied     = 0;
    role            = FOLLOWER;
    electionTimeout = generateTimeout();
    lastTimeout     = millis();

    Serial.println("Initialized ServerState");

    Serial.printf("%-25s%-25s%-25s%-25s\n",
                  "currentTerm",
                  "votedFor",
                  "electionTimeout",
                  "lastTimeout"
                  );
    Serial.printf("%-25lu%-25lu%-25lu%-25lu",
                  currentTerm,
                  votedFor,
                  electionTimeout,
                  lastTimeout
                  );
}

RequestVoteResponse ServerState::handleRequestVoteReq(uint32_t term,
                                                      uint32_t candidateID,
                                                      uint32_t lastLogIndex,
                                                      uint32_t lastLogTerm) {
    Serial.println("------handleRequestVoteRCP------");
    Serial.println(term);
    Serial.println(candidateID);
    Serial.println(lastLogIndex);
    Serial.println(lastLogTerm);
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
    return handleRequestVoteReq(msg.term,
                                msg.candidateID,
                                msg.lastLogIndex,
                                msg.lastLogTerm);
}

void ServerState::handleRequestVoteRes(uint32_t term,
                                       uint8_t  voteGranted) {
    Serial.printf("RVRes. term: %d, voteGranted: %d, role: %d\n",
                  term,
                  voteGranted,
                  role);

    if (role == CANDIDATE) {
        if (voteGranted) {
            receivedVotes++;
            checkGrantedVotes();
        } else {
            if (currentTerm < term) { // this condition might be superfluous
                currentTerm = term;
                role        = FOLLOWER;
            }
        }
    } // ELSE ignore obsolete packet
}

void ServerState::handleRequestVoteRes(RequestVoteResponse msg) {
    return handleRequestVoteRes(msg.term,
                                msg.voteGranted);
}

RequestVoteRequest ServerState::checkElectionTimeout() {
    RequestVoteRequest msg;

    if (millis() > lastTimeout + electionTimeout) {
        Serial.println("\n[WARN] Election timout ---> starting a new election");
        role = CANDIDATE;
        currentTerm++;

        // starting a new election always results in first voting for
        // itself
        receivedVotes    = 1;
        votedFor         = selfID;
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
    if (requiredVotes <= receivedVotes) {
        Serial.printf("ELECTED LEADER! Received %d votes\n", receivedVotes);
        role             = LEADER;
        heartbeatTimeout = random(100, 150);
        lastTimeout      = millis();
        Serial.printf("New heartbeatTimeout: %lu\ncurrent millis: %lu\n",
                      heartbeatTimeout,
                      lastTimeout);
    }
    Serial.printf("required: %d, received %d\n", requiredVotes, receivedVotes);
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
    if (millis() > lastTimeout + heartbeatTimeout) {
        lastTimeout = millis();
        return 1;
    }
    return 0;
}

uint32_t ServerState::getCurrentTerm() {
    return currentTerm;
}
