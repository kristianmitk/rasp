#include "Arduino.h"
#include "messages.h"
#include "marshall.h"
#define PACKET_BODY_OFFSET 1

// TODO: marshalling should return stack buffer instead of buffer pointer

/**
 * --------------------------- RequestVoteRequest ---------------------------
 */

RequestVoteRequest::RequestVoteRequest() {
    this->type        = Message::RequestVoteReq;
    this->candidateID = 0;
}

RequestVoteRequest::RequestVoteRequest(uint8_t *packet) {
    this->type = Message::RequestVoteReq;

    this->term         = unpack_uint32_t(packet, PACKET_BODY_OFFSET);
    this->candidateID  = unpack_uint32_t(packet, PACKET_BODY_OFFSET + 4);
    this->lastLogIndex = unpack_uint32_t(packet, PACKET_BODY_OFFSET + 8);
    this->lastLogTerm  = unpack_uint32_t(packet, PACKET_BODY_OFFSET + 12);
}

uint8_t * RequestVoteRequest::marshall() {
    uint8_t *buffer = new uint8_t[REQ_VOTE_REQ_MSG_SIZE];

    pack_uint8_t(buffer, 0, Message::RequestVoteReq);

    pack_uint32_t(buffer, PACKET_BODY_OFFSET,      this->term);
    pack_uint32_t(buffer, PACKET_BODY_OFFSET + 4,  this->candidateID);
    pack_uint32_t(buffer, PACKET_BODY_OFFSET + 8,  this->lastLogIndex);
    pack_uint32_t(buffer, PACKET_BODY_OFFSET + 12, this->lastLogTerm);
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
 * --------------------------- RequestVoteResponse ---------------------------
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

    pack_uint8_t(buffer, 0, RequestVoteRes);
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

Message* a(uint8_t *packet) {
    rvReq = RequestVoteRequest(packet);
    return &rvReq;
}

Message* b(uint8_t *packet) {
    rvRes = RequestVoteResponse(packet);
    return &rvReq;
}

Message * (*messageExtractors[2])(uint8_t * packet) = { a, b };

Message* createMessage(uint8_t *packet) {
    Serial.printf("\n\n%d\n", unpack_uint8_t(packet, 0));
    return messageExtractors[unpack_uint8_t(packet, 0)](packet);
}
