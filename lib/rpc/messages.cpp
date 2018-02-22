#include "Arduino.h"
#include "messages.h"
#include "marshall.h"
#define PACKET_BODY_OFFSET 1
#define PACKET_BODY_DATA_OFFSET PACKET_BODY_OFFSET + 16

// TODO: marshalling should return stack buffer instead of buffer pointer
//      -> how should this work?

/**
 * ----------------------------- RequestVoteRequest ----------------------------
 */

RequestVoteRequest::RequestVoteRequest() {
    this->type        = Message::RequestVoteReq;
    this->candidateID = 0;
}

RequestVoteRequest::RequestVoteRequest(uint8_t *packet) {
    this->type = Message::RequestVoteReq;

    this->term         = unpack_uint32_t(packet, PACKET_BODY_OFFSET);
    this->candidateID  = unpack_uint32_t(packet, PACKET_BODY_OFFSET + 4);
    this->lastLogIndex = unpack_uint16_t(packet, PACKET_BODY_OFFSET + 8);
    this->lastLogTerm  = unpack_uint32_t(packet, PACKET_BODY_OFFSET + 10);
}

uint8_t * RequestVoteRequest::marshall() {
    uint8_t *buffer = new uint8_t[REQ_VOTE_REQ_MSG_SIZE];

    pack_uint8_t(buffer, 0, Message::RequestVoteReq);
    pack_uint32_t(buffer, PACKET_BODY_OFFSET,     this->term);
    pack_uint32_t(buffer, PACKET_BODY_OFFSET + 4, this->candidateID);
    pack_uint16_t(buffer, PACKET_BODY_OFFSET + 8, this->lastLogIndex);
    pack_uint32_t(buffer, PACKET_BODY_OFFSET + 10, this->lastLogTerm);
    return buffer;
}

void RequestVoteRequest::serialPrint() {
    Serial.printf("RequestVote request message\n%-25s%-25s%-25s%-25s\n",
                  "term",
                  "candidateID",
                  "lastLogIndex",
                  "lastTogTerm"
                  );
    Serial.printf("%-25lu%-25lu%-25lu%-25lu\n",
                  this->term,
                  this->candidateID,
                  this->lastLogIndex,
                  this->lastLogTerm
                  );
}

size_t RequestVoteRequest::size() {
    return REQ_VOTE_REQ_MSG_SIZE;
}

/**
 * ---------------------------- RequestVoteResponse ----------------------------
 */


RequestVoteResponse::RequestVoteResponse() {
    this->type = Message::RequestVoteRes;
}

RequestVoteResponse::RequestVoteResponse(uint8_t *packet) {
    this->type        = Message::RequestVoteRes;
    this->term        = unpack_uint32_t(packet, PACKET_BODY_OFFSET);
    this->voteGranted = unpack_uint8_t(packet, PACKET_BODY_OFFSET + 4);
}

uint8_t * RequestVoteResponse::marshall() {
    uint8_t *buffer = new uint8_t[REQ_VOTE_RES_MSG_SIZE];

    pack_uint8_t(buffer, 0, Message::RequestVoteRes);
    pack_uint32_t(buffer, PACKET_BODY_OFFSET, this->term);
    pack_uint8_t(buffer, PACKET_BODY_OFFSET + 4, this->voteGranted);
    return buffer;
}

void RequestVoteResponse::serialPrint() {
    Serial.printf("RequestVote response message:\n%-25s%-25s\n",
                  "term",
                  "voteGranted"
                  );
    Serial.printf("%-25lu%-25d\n",
                  this->term,
                  this->voteGranted
                  );
}

size_t RequestVoteResponse::size() {
    return REQ_VOTE_RES_MSG_SIZE;
}

/**
 * --------------------------- AppendEntriesRequest ----------------------------
 */

AppendEntriesRequest::AppendEntriesRequest() {
    this->type     = Message::AppendEntriesReq;
    this->dataSize = 0;
}

AppendEntriesRequest::AppendEntriesRequest(uint8_t *packet, uint16_t size) {
    this->type         = Message::AppendEntriesReq;
    this->term         = unpack_uint32_t(packet, PACKET_BODY_OFFSET);
    this->leaderId     = unpack_uint32_t(packet, PACKET_BODY_OFFSET + 4);
    this->prevLogIndex = unpack_uint16_t(packet, PACKET_BODY_OFFSET + 8);
    this->prevLogTerm  = unpack_uint32_t(packet, PACKET_BODY_OFFSET + 10);
    this->leaderCommit = unpack_uint16_t(packet, PACKET_BODY_OFFSET + 14);

    // TODO: better doc, its lateee and I'm yawning
    // We dont need to send the dataSize as we can deduce this from packetSize
    // and empty heartbeat message (i.e everything in the message except the
    // data)
    this->dataSize = size - EMPTY_HEARTBEAT_MSG_SIZE;

    if (this->dataSize) this->data = packet + PACKET_BODY_OFFSET + 16;
    else this->data = NULL;
}

uint8_t * AppendEntriesRequest::marshall() {
    uint8_t *buffer = new uint8_t[PACKET_BODY_DATA_OFFSET + this->dataSize];

    pack_uint8_t(buffer, 0, Message::AppendEntriesReq);
    pack_uint32_t(buffer, PACKET_BODY_OFFSET,     this->term);
    pack_uint32_t(buffer, PACKET_BODY_OFFSET + 4, this->leaderId);
    pack_uint16_t(buffer, PACKET_BODY_OFFSET + 8, this->prevLogIndex);
    pack_uint32_t(buffer, PACKET_BODY_OFFSET + 10, this->prevLogTerm);
    pack_uint16_t(buffer, PACKET_BODY_OFFSET + 14, this->leaderCommit);

    if (this->dataSize) memcpy(&buffer[PACKET_BODY_DATA_OFFSET],
                               this->data,
                               this->dataSize);
    return buffer;
}

void AppendEntriesRequest::serialPrint() {
    Serial.printf("AppendEntries request message \
                \n%-20s%-20s%-20s%-20s%-20s\n",
                  "term",
                  "leaderId",
                  "prevLogIndex",
                  "prevLogTerm",
                  "leaderCommit"
                  );
    Serial.printf("%-20lu%-20lu%-20lu%-20lu%-20lu\n",
                  this->term,
                  this->leaderId,
                  this->prevLogIndex,
                  this->prevLogTerm,
                  this->leaderCommit
                  );
    Serial.printf("Data Size is: %lub\n", this->dataSize);
}

size_t AppendEntriesRequest::size() {
    return EMPTY_HEARTBEAT_MSG_SIZE;
}

/**
 * --------------------------- AppendEntriesResponse ---------------------------
 */

AppendEntriesResponse::AppendEntriesResponse() {
    this->type = Message::AppendEntriesRes;
}

AppendEntriesResponse::AppendEntriesResponse(uint8_t *packet) {
    this->type     = Message::AppendEntriesRes;
    this->term     = unpack_uint32_t(packet, PACKET_BODY_OFFSET);
    this->success  = unpack_uint8_t(packet, PACKET_BODY_OFFSET + 4);
    this->serverId = unpack_uint32_t(packet, PACKET_BODY_OFFSET + 5);
}

uint8_t * AppendEntriesResponse::marshall() {
    uint8_t *buffer = new uint8_t[APP_ENTRIES_RES_MSG_SIZE];

    pack_uint8_t(buffer, 0, Message::AppendEntriesRes);
    pack_uint32_t(buffer, PACKET_BODY_OFFSET, this->term);
    pack_uint8_t(buffer, PACKET_BODY_OFFSET + 4, this->success);
    pack_uint32_t(buffer, PACKET_BODY_OFFSET + 5, this->serverId);

    return buffer;
}

void AppendEntriesResponse::serialPrint() {
    Serial.printf("AppendEntries response message \
                \n%-25s%-25s%-25s\n",
                  "term",
                  "success",
                  "senderId"
                  );
    Serial.printf("%-25lu%-25lu%-25lu\n",
                  this->term,
                  this->success,
                  this->serverId
                  );
}

size_t AppendEntriesResponse::size() {
    return APP_ENTRIES_RES_MSG_SIZE;
}

/**
 * ---------------------------         OTHER         ---------------------------
 */
Message* a(uint8_t *packet, uint16_t size) {
    rvReq = RequestVoteRequest(packet);
    return &rvReq;
}

Message* b(uint8_t *packet, uint16_t size) {
    rvRes = RequestVoteResponse(packet);
    return &rvRes;
}

Message* c(uint8_t *packet, uint16_t size) {
    aeReq = AppendEntriesRequest(packet, size);
    return &aeReq;
}

Message* d(uint8_t *packet, uint16_t size) {
    aeRes = AppendEntriesResponse(packet);
    return &aeRes;
}

/**
 * TODO: DOCS
 * NOTE: keep the order like we have it in Message::MessageType
 * [createMessage description]
 * @param  packet [description]
 * @return        [description]
 */
Message * (*messageExtractors[4])(uint8_t * packet,
                                  uint16_t size) = { a, b, c, d };

Message* createMessage(uint8_t *packet, uint16_t size) {
    uint8_t messageType = unpack_uint8_t(packet, 0);

    Serial.printf("MessageType: %d\n", messageType);

    return messageType < Message::lastVal ?
           messageExtractors[messageType](packet, size) : NULL;
}
