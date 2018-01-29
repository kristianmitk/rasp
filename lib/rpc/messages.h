#ifndef messages_h

#include "util.h"
#define messages_h
extern "C" {
    #include <stdint.h>
}

#define REQ_VOTE_REQ_MSG_SIZE 20
#define REQ_VOTE_RES_MSG_SIZE 9

/**
 * This enum is used to identify the message. Every incoming packet
 * specifies
 * on its first 4 bytes the message type.
 */
enum MessageType {
    RequestVoteReq   = 101,
    RequestVoteRes   = 102,
    AppendEntriesReq = 201,
    AppendEntriesRes = 202
};

/**
 * TODO: DOCS
 */
class Message {
public:

    /**
     * TODO: DOCS
     * [marshall description]
     * @return [description]
     */
    virtual uint8_t* marshall()    = 0;
    virtual void     serialPrint() = 0;
};

/**
 * TODO: DOCS
 */
class RequestVoteRequest : public Message {
public:

    operator int() const
    { return candidateID; }

    uint32_t term;
    uint32_t candidateID;
    uint32_t lastLogIndex;
    uint32_t lastLogTerm;

    /**
     * TODO: DOCS
     * [RequestVoteRequestMessage description]
     * @param packet [description]
     */
    RequestVoteRequest(uint8_t *packet);
    RequestVoteRequest();


    virtual uint8_t* marshall();
    virtual void     serialPrint();
};

/**
 * TODO: DOCS
 */
class RequestVoteResponse : public Message {
public:

    uint32_t term;
    bool voteGranted;

    /**
     * TODO: DOCS
     * [RequestVoteResponseMessage description]
     * @param packet [description]
     */
    RequestVoteResponse(uint8_t *packet);
    RequestVoteResponse();

    /**
     * TODO: DOCS
     * [RequestVoteResponse description]
     * @param term        [description]
     * @param voteGranted [description]
     */
    RequestVoteResponse(uint32_t term,
                        uint8_t  voteGranted);

    virtual uint8_t* marshall();
    virtual void     serialPrint();
};

/**
 * TODO: DOCS
 */
class AppendEntriesRequestMessage : public Message {
    // TODO: add missing properties
};

/**
 * TODO: DOCS
 */
class AppendEntriesResponseMessage : public Message {
    // TODO: add missing properties
};


#endif // ifndef messages_h
