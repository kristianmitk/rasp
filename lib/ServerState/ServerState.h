#ifndef ServerState_h
#define ServerState_h

#include "Arduino.h"
#include "Log.h"
#include "messages.h"
#include "rasp_nodes.h"
#include "rasp_fs2.h"
extern "C" {
    #include <stdint.h>
}

enum Role {
    FOLLOWER,
    CANDIDATE,
    LEADER
};

/**
 * TODO: DOCS
 * TODO: Change to Singleton pattern
 * [ServerState description]
 * @param id [description]
 */
class ServerState {
public:

    ServerState(uint32_t id);

    void                serialPrint();

    /**
     * TODO: DOCS
     * [checkElectionTimeout description]
     * @return [description]
     */
    RequestVoteRequest  checkElectionTimeout();

    /**
     * TODO: DOCS
     * [handleRequestVoteRPC description]
     * @param  term         [description]
     * @param  candidateID  [description]
     * @param  lastLogIndex [description]
     * @param  lastLogTerm  [description]
     * @return              [description]
     */
    RequestVoteResponse handleRequestVoteReq(uint32_t term,
                                             uint32_t scandidateID,
                                             uint32_t lastLogIndex,
                                             uint32_t lastLogTerm);

    /**
     * TODO: DOCS
     * [handleRequestVoteRPC description]
     * @param  message [description]
     * @return         [description]
     */
    RequestVoteResponse handleRequestVoteReq(RequestVoteRequest message);

    /**
     * TODO: DOCS
     * [handleRequestVoteRes description]
     * @param  term        [description]
     * @param  voteGranted [description]
     * @return             [description]
     */
    void                handleRequestVoteRes(uint32_t term,
                                             uint8_t  voteGranted);

    /**
     * TODO: DOCS
     * [handleRequestVoteRes description]
     * @param  msg [description]
     * @return     [description]
     */
    void handleRequestVoteRes(RequestVoteResponse msg);

    /**
     * TODO: DOCS
     * [resetElectionTimeout description]
     */
    void resetElectionTimeout();

    /**
     * TODO: DOCS
     * [checkHeartbeatTimeout description]
     * @return [description]
     */
    void resetElectionTimeout(uint8_t placeholder);


    /**
     * TODO: DOCS
     * [getRole description]
     * @return [description]
     */
    Role    getRole();

    /**
     * TODO: DOCS
     * [checkHeartbeatTimeout description]
     * @return [description]
     */
    uint8_t checkHeartbeatTimeout();

private:

    /**
     * TODO: DOCS
     * [ServerState::checkGrantedVotes description]
     */
    void checkGrantedVotes();

    // own chipID used as an ID to identify a server
    uint32_t selfID;

    // needed in order to check for electionTimeout (compared to millis())
    uint32_t lastTimeout;

    // to count the number of recieved votes
    uint8_t receivedVotes;

    /* ------------------ RAFT DEFINED PROPS ------------------ */
    Role role;

    // -------- persistent state
    //          TODO: everywhere this variables are modified make this through a
    //                function which first writes to disk
    uint32_t currentTerm;

    // TODO: is here a string a better solution?
    uint32_t votedFor;
    Log *log;

    // -------- volatile state
    uint32_t commitIndex;
    uint32_t lastApplied;

    // -------- volatile state as leader
    uint32_t nextIndex[];
    uint32_t matchIndex[];

    uint16_t electionTimeout;

    uint16_t heartbeatTimeout;
};

#endif // ifndef ServerState_h
