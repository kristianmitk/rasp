#include "Arduino.h"
#include "messages.h"
#include "marshall.h"
#define PACKET_BODY_OFFSET 4

/**
 * --------------------------- RequestVoteRequest ---------------------------
 */

RequestVoteRequest::RequestVoteRequest() {
    this->candidateID = 0;
}

RequestVoteRequest::RequestVoteRequest(uint8_t *packet) {
    this->term         = unpack_uint32_t(packet, PACKET_BODY_OFFSET);
    this->candidateID  = unpack_uint32_t(packet, PACKET_BODY_OFFSET + 4);
    this->lastLogIndex = unpack_uint32_t(packet, PACKET_BODY_OFFSET + 8);
    this->lastLogTerm  = unpack_uint32_t(packet, PACKET_BODY_OFFSET + 12);
}

uint8_t * RequestVoteRequest::marshall() {
    uint8_t *buffer = new uint8_t[REQ_VOTE_REQ_MSG_SIZE];

    pack_uint32_t(buffer,  0, RequestVoteReq);
    pack_uint32_t(buffer,  4, this->term);
    pack_uint32_t(buffer,  8, this->candidateID);
    pack_uint32_t(buffer, 12, this->lastLogIndex);
    pack_uint32_t(buffer, 16, this->lastLogTerm);
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


RequestVoteResponse::RequestVoteResponse() {}

RequestVoteResponse::RequestVoteResponse(uint8_t *packet) {
    this->term        = unpack_uint32_t(packet, PACKET_BODY_OFFSET);
    this->voteGranted = unpack_uint8_t(packet, PACKET_BODY_OFFSET + 4);
}

uint8_t * RequestVoteResponse::marshall() {
    uint8_t *buffer = new uint8_t[REQ_VOTE_RES_MSG_SIZE];

    pack_uint32_t(buffer, 0, RequestVoteRes);
    pack_uint32_t(buffer, 4, this->term);
    pack_uint8_t(buffer, 8, this->voteGranted);
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
