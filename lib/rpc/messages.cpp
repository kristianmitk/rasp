#include "Arduino.h"
#include "messages.h"
#include "marshall.h"
#define PACKET_BODY_OFFSET 1

// TODO: marshalling should return stack buffer instead of buffer pointer

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
    this->lastLogIndex = unpack_uint8_t(packet, PACKET_BODY_OFFSET + 8);
    this->lastLogTerm  = unpack_uint32_t(packet, PACKET_BODY_OFFSET + 9);
}

uint8_t * RequestVoteRequest::marshall() {
    uint8_t *buffer = new uint8_t[REQ_VOTE_REQ_MSG_SIZE];

    pack_uint8_t(buffer, 0, Message::RequestVoteReq);
    pack_uint32_t(buffer, PACKET_BODY_OFFSET,     this->term);
    pack_uint32_t(buffer, PACKET_BODY_OFFSET + 4, this->candidateID);
    pack_uint8_t(buffer, PACKET_BODY_OFFSET + 8, this->lastLogIndex);
    pack_uint32_t(buffer, PACKET_BODY_OFFSET + 9, this->lastLogTerm);
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

/**
 * --------------------------- AppendEntriesResponse ---------------------------
 */

AppendEntriesRequest::AppendEntriesRequest() {
    this->type = Message::AppendEntriesReq;

    // TODO: add missing stuff
}

AppendEntriesRequest::AppendEntriesRequest(uint8_t *packet) {
    this->type = Message::AppendEntriesReq;

    // TODO: add missing stuff
}

uint8_t * AppendEntriesRequest::marshall() {
    // TODO implement
    return NULL;
}

void AppendEntriesRequest::serialPrint() {
    // TODO: implement
}

/**
 * ---------------------------         OTHER         ---------------------------
 */
Message* a(uint8_t *packet) {
    rvReq = RequestVoteRequest(packet);
    return &rvReq;
}

Message* b(uint8_t *packet) {
    rvRes = RequestVoteResponse(packet);
    return &rvRes;
}

Message* c(uint8_t *packet) {
    aeReq = AppendEntriesRequest(packet);
    return &aeReq;
}

/**
 * TODO: DOCS
 * NOTE: keep the order like we have it in Message::MessageType
 * [createMessage description]
 * @param  packet [description]
 * @return        [description]
 */
Message * (*messageExtractors[3])(uint8_t * packet) = { a, b, c };

Message* createMessage(uint8_t *packet) {
    uint8_t messageType = unpack_uint8_t(packet, 0);

    Serial.printf("\nMessageType: %d\n", messageType);

    return messageType < Message::lastVal ?
           messageExtractors[messageType](packet) : NULL;
}
