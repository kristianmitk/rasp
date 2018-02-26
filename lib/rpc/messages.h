#ifndef messages_h
#include <Arduino.h>

#include "marshall.h"
#define messages_h
extern "C" {
    #include <stdint.h>
}

#define REQ_VOTE_REQ_MSG_SIZE 15
#define REQ_VOTE_RES_MSG_SIZE 6
#define EMPTY_HEARTBEAT_MSG_SIZE 21
#define APP_ENTRIES_RES_MSG_SIZE 12

/**
 * TODO: DOCS
 */
class Message {
public:

    /**
     * This enum is used to identify the message type. Every incoming packet
     * specifies on its first byte the message type.
     */
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
    virtual uint8_t* marshall() = 0;

    /**
     * TODO: DOCS
     * [serialPrint description]
     * @return [description]
     */
    virtual void     serialPrint() = 0;

    /**
     * TODO: DOCS
     * [size description]
     * @return [description]
     */
    virtual size_t   size() = 0;
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
    uint16_t lastLogIndex;
    uint32_t lastLogTerm;

    RequestVoteRequest();

    RequestVoteRequest(uint8_t *packet);

    virtual uint8_t* marshall();
    virtual void     serialPrint();
    virtual size_t   size();
};

/**
 * TODO: DOCS
 */
class RequestVoteResponse : public Message {
public:

    uint32_t term;
    bool voteGranted;

    RequestVoteResponse();
    RequestVoteResponse(uint32_t term,
                        uint8_t  voteGranted);
    RequestVoteResponse(uint8_t *packet);

    virtual uint8_t* marshall();
    virtual void     serialPrint();
    virtual size_t   size();
};

/**
 * TODO: DOCS
 */
class AppendEntriesRequest : public Message {
public:

    uint32_t term;
    uint32_t leaderId;
    uint16_t prevLogIndex;
    uint32_t prevLogTerm;
    uint16_t leaderCommit;
    uint32_t dataTerm;
    uint8_t *data;
    uint16_t dataSize;

    AppendEntriesRequest();

    // we assume there is no data (log entries) to be send -> we dont need to
    // pass the size of the data part
    AppendEntriesRequest(uint8_t *packet);
    AppendEntriesRequest(uint8_t *packet,
                         uint16_t size);

    virtual uint8_t* marshall();
    virtual void     serialPrint();
    virtual size_t   size();
};

/**
 * TODO: DOCS
 */
class AppendEntriesResponse : public Message {
public:

    uint32_t term;
    uint8_t success;
    uint16_t matchIndex;

    // TODO: remove this and use the sender IP at receiver side instead?
    // this is not in the Raft specification - we use this so we can easily
    // identify to whom the message belongs inside the ServerState handler
    uint32_t serverId;

    // TODO: add matchIndex
    // uint16_t matchIndex;

    AppendEntriesResponse();
    AppendEntriesResponse(uint8_t *packet);

    virtual uint8_t* marshall();
    virtual void     serialPrint();
    virtual size_t   size();
};


/**
 * This static objects are used all over the single routine that is running on
 * the ESP8266 boards. Received packets pass its data to the proper message type
 * and between function calls the pointers to this objects are given to the
 * callee. Vice versa: packets to send are created out of this objects
 */

static RequestVoteRequest    rvReq;
static RequestVoteResponse   rvRes;
static AppendEntriesRequest  aeReq;
static AppendEntriesResponse aeRes;

Message* createMessage(uint8_t *packet,
                       uint16_t size);

#endif // ifndef messages_h
