#ifndef messages_h

#include "util.h"
#include "marshall.h"
#define messages_h
extern "C" {
    #include <stdint.h>
}

#define REQ_VOTE_REQ_MSG_SIZE 14
#define REQ_VOTE_RES_MSG_SIZE 6

/**
 * This enum is used to identify the message. Every incoming packet
 * specifies
 * on its first 4 bytes the message type.
 */

/**
 * TODO: DOCS
 */
class Message {
public:

    enum type: uint8_t {
        RequestVoteReq   = 0,
        RequestVoteRes   = 1,
        AppendEntriesReq = 2,
        AppendEntriesRes = 3,

        // to find out when we bounce out of index in
        // `createMessage(uint8_t *packet)`
        lastVal
    } type;

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
    uint8_t lastLogIndex;
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
class AppendEntriesRequest : public Message {
public:

    // TODO: add missing properties

    /**
     * TODO: DOCS
     * [AppendEntriesRequest description]
     * @param packet [description]
     */
    AppendEntriesRequest(uint8_t *packet);
    AppendEntriesRequest();
    virtual uint8_t* marshall();
    virtual void     serialPrint();
};

/**
 * TODO: DOCS
 */
class AppendEntriesResponse : public Message {
    // TODO: add missing properties

    virtual uint8_t* marshall();
    virtual void     serialPrint();
};

// TODO: DOCUMENT WHATS GOING ON HERE - TOO LATE NOW ':D


static RequestVoteRequest   rvReq;
static RequestVoteResponse  rvRes;
static AppendEntriesRequest aeReq;

// AppendEntriesResponse aeRes;


Message* a(uint8_t *packet); // TODO: rename function names

Message* b(uint8_t *packet); // TODO: rename function names

Message* c(uint8_t *packet); // TODO: rename function names

Message* createMessage(uint8_t *packet);

#endif // ifndef messages_h
