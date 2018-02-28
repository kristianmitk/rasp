#ifndef ServerState_h
#define ServerState_h

#include "Arduino.h"
#include "common.h"
#include "messages.h"
#include "rasp_nodes.h"
#include "rasp_fs.h"
#include "UDPServer.h"

#define NUM_FOLLOWERS RASP_NUM_SERVERS - 1

// TODO: remove redundancies with `rasp_nodes.h` server array
typedef struct followerState {
    uint8_t  IP[4];
    uint32_t id;
    uint16_t nextIndex;
    uint16_t matchIndex;
    uint32_t lastTimeout;
} followerState_t;


/**
 * TODO: DOCS
 * TODO: Change to Singleton pattern
 * [ServerState description]
 * @param id [description]
 */
class ServerState {
public:

    /**
     * TODO: DOCS
     * [ServerState description]
     * @param id [description]
     */
    enum Role {
        FOLLOWER,
        CANDIDATE,
        LEADER
    };

    static ServerState& getInstance()
    {
        static ServerState instance;

        return instance;
    }

    /**
     * This function handles actually what the empty constructor could do.
     * But since the ServerState is in the global scope of the `main.cpp`
     * function and at the moment we initialize this object the Serial class is
     * not started (i.e Serial.begin() needs to be executed) there is nothing
     * printed to the serial monitor. Because of that we use this separate
     * function and call it after Serial.begin() was successfully executed.
     */
    void initialize();

    /**
     * TODO: DOCS
     * [serialPrint description]
     * @return [description]
     */
    void serialPrint();

    /**
     * TODO: DOCS
     * [loopHandler description]
     */
    void loopHandler();


    /**
     * TODO: DOCS
     * [getRole description]
     * @return [description]
     */
    Role     getRole();

    uint32_t getCurrentTerm() {
        return this->currentTerm;
    }

private:

    /**
     * TODO: DOCS
     * [dispatch description]
     * @param  msg [description]
     * @return     [description]
     */
    void handleMessage();

    /**
     * TODO: DOCS
     * [checkElectionTimeout description]
     * @return [description]
     */
    void checkElectionTimeout();


    /**
     * TODO: DOCS
     * [handleRequestVoteRPC description]
     * @param  term         [description]
     * @param  candidateID  [description]
     * @param  lastLogIndex [description]
     * @param  lastLogTerm  [description]
     * @return              [description]
     */
    Message* handleRequestVoteReq(uint32_t term,
                                  uint32_t scandidateID,
                                  uint16_t lastLogIndex,
                                  uint32_t lastLogTerm);

    /**
     * TODO: DOCS
     * [handleRequestVoteRPC description]
     * @param  message [description]
     * @return         [description]
     */
    Message* handleRequestVoteReq(Message *message);

    /**
     * TODO: DOCS
     * [handleRequestVoteRes description]
     * @param  term        [description]
     * @param  voteGranted [description]
     * @return             [description]
     */
    void     handleRequestVoteRes(uint32_t term,
                                  uint8_t  voteGranted);

    /**
     * TODO: DOCS
     * [handleRequestVoteRes description]
     * @param  msg [description]
     * @return     [description]
     */
    void handleRequestVoteRes(Message *msg);


    /**
     * TODO: DOCS
     * [handleAppendEntriesReq description]
     * @param  msg [description]
     * @return     [description]
     */
    Message* handleAppendEntriesReq(Message *msg);

    /**
     * TODO: DOCS
     * [handleAppendEntriesRes description]
     * @param  msg [description]
     * @return     [description]
     */
    void     handleAppendEntriesRes(Message *msg);


    /**
     * TODO: DOCS
     * [resetElectionTimeout description]
     */
    void             resetElectionTimeout();

    /**
     * TODO: DOCS
     * [checkHeartbeatTimeout description]
     * @return [description]
     */
    void             checkHeartbeatTimeouts();

    void             DEBUG_APPEND_LOG();

    /**
     * TODO: DOCS
     * [ServerState::checkForNewCommitedIndex description]
     */
    void             checkForNewCommitedIndex();

    /**
     * TODO: DOCS
     * [getFollower description]
     * @param  id [description]
     * @return    [description]
     */
    followerState_t* getFollower(uint32_t id);

    /**
     * TODO: DOCS
     * [ServerState::checkGrantedVotes description]
     */
    void             checkGrantedVotes();

    // to redirect clients when they request a leader
    uint32_t leaderId;

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

    // -------- volatile state
    uint32_t commitIndex;
    uint32_t lastApplied;

    // -------- volatile state as leader
    followerState_t followerStates[NUM_FOLLOWERS];

    uint16_t electionTimeout;
    uint16_t heartbeatTimeout;


    /**
     * TODO: DOCS
     * [ServerState description]
     * @param id [description]
     */
    ServerState() {}


    ServerState(ServerState const&);
    void operator=(ServerState const&);
};

#endif // ifndef ServerState_h
