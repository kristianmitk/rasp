#include "ServerState.h"
#include "util.h"

// TODO: outsource boundaries to a config file
#define MIN_ELECTION_TIMEOUT 200
#define MAX_ELECTION_TIMEOUT 350
#define generateTimeout() random(MIN_ELECTION_TIMEOUT, MAX_ELECTION_TIMEOUT)

#define REQUIRED_VOTES (RASP_NUM_SERVERS / 2 + 1)

ServerState::ServerState(uint32_t id) {
    selfID          = id;
    receivedVotes   = 0;
    currentTerm     = RASPFS::getInstance().read(RASPFS::CURRENT_TERM);
    votedFor        = RASPFS::getInstance().read(RASPFS::VOTED_FOR);
    commitIndex     = 0;
    lastApplied     = 0;
    log             = new Log();
    role            = FOLLOWER;
    electionTimeout = generateTimeout();
    lastTimeout     = millis();

    this->serialPrint();
    Serial.println();
}

Message * ServerState::dispatch(Message *msg) {
    switch (msg->type) {
    case Message::RequestVoteReq:
        return handleRequestVoteReq(msg);

    case Message::RequestVoteRes:
        handleRequestVoteRes(msg);
        return NULL;

    case Message::AppendEntriesReq:
        Serial.println("<3");
        resetElectionTimeout(1);
        return NULL;

    case Message::AppendEntriesRes:

        // TODO: implement
        break;
    }
    return NULL;
}

// TODO: set persistent values
Message * ServerState::handleRequestVoteReq(uint32_t term,
                                            uint32_t candidateID,
                                            uint8_t  lastLogIndex,
                                            uint32_t lastLogTerm) {
    rvRes.term        = currentTerm;
    rvRes.voteGranted = false;

    // our state is more up to date than the candidate state
    if (term <= this->currentTerm) {
        Serial.println("VoteGranted = false");
        return &rvRes;
    }

    // if (!this->votedFor || (this->votedFor != selfID) ||
    //     ((this->role == CANDIDATE) && (this->currentTerm < term)))
    if (!this->votedFor || (this->currentTerm < term)) {
        currentTerm = RASPFS::getInstance().write(RASPFS::CURRENT_TERM, term);
        rvRes.term  = term;

        // SAFETY RULES (§5.2, §5.4)
        if ((lastLogTerm >= log->lastStoredTerm()) &&
            (lastLogIndex >= log->lastIndex()))
        {
            Serial.println("VoteGranted = true");

            role     = FOLLOWER;
            votedFor = RASPFS::getInstance().write(RASPFS::VOTED_FOR,
                                                   candidateID);
            rvRes.voteGranted = true;

            resetElectionTimeout();
        }
    } else {
        Serial.printf("Self candidate in term: %d? %d\n",
                      term,
                      ((this->role == CANDIDATE) &&
                       (this->currentTerm == term)));
        Serial.println("VoteGranted = false - 2nd condition");
    }
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

void ServerState::handleRequestVoteRes(Message *msg) {
    RequestVoteResponse *p = (RequestVoteResponse *)msg;

    p->serialPrint();
    return handleRequestVoteRes(p->term,
                                p->voteGranted);
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
        votedFor      = RASPFS::getInstance().write(RASPFS::VOTED_FOR, selfID);
        receivedVotes = 1;

        rvReq.term         = this->currentTerm;
        rvReq.candidateID  = this->selfID;
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
        role             = LEADER;
        heartbeatTimeout = 75; // random(50, 100)
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

ServerState::Role ServerState::getRole() {
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
            uint16_t entrySize = random(1, 50);
            uint8_t  entryData[entrySize];
            entryData[0] = random(0, 2);

            this->log->append(currentTerm, entryData, entrySize);
            this->log->printLastEntry();
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
