#include "messages.h"

RequestVoteRequest    rvReq = RequestVoteRequest();
RequestVoteResponse   rvRes = RequestVoteResponse();
AppendEntriesRequest  aeReq = AppendEntriesRequest();
AppendEntriesResponse aeRes = AppendEntriesResponse();
StateMachineMessage   smMsg = StateMachineMessage();


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
#if PRINT_DEBUG
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
#endif // if PRINT_DEBUG
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
#if PRINT_DEBUG
    RASPDBG("RequestVote response message:\n%-25s%-25s\n",
            "term",
            "voteGranted"
            )

    Serial.printf("%-25lu%-25d\n",
                  this->term,
                  this->voteGranted
                  );
#endif // if PRINT_DEBUG
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

    // TODO: optimize this by putting it into the data block and not sending
    // unnecessary 4 bytes in a empty heartbeat message
    this->dataTerm = unpack_uint32_t(packet, PACKET_BODY_OFFSET + 16);

    // TODO: better doc, its lateee and I'm yawning
    // We dont need to send the dataSize as we can deduce this from packetSize
    // and empty heartbeat message (i.e everything in the message except the
    // data)
    this->dataSize = size - EMPTY_HEARTBEAT_MSG_SIZE;

    this->data = this->dataSize ? packet + EMPTY_HEARTBEAT_MSG_SIZE : NULL;
}

uint8_t * AppendEntriesRequest::marshall() {
    uint8_t *buffer = new uint8_t[EMPTY_HEARTBEAT_MSG_SIZE + this->dataSize];

    pack_uint8_t(buffer, 0, Message::AppendEntriesReq);
    pack_uint32_t(buffer, PACKET_BODY_OFFSET,     this->term);
    pack_uint32_t(buffer, PACKET_BODY_OFFSET + 4, this->leaderId);
    pack_uint16_t(buffer, PACKET_BODY_OFFSET + 8, this->prevLogIndex);
    pack_uint32_t(buffer, PACKET_BODY_OFFSET + 10, this->prevLogTerm);
    pack_uint16_t(buffer, PACKET_BODY_OFFSET + 14, this->leaderCommit);
    pack_uint32_t(buffer, PACKET_BODY_OFFSET + 16, this->dataTerm);

    if (this->dataSize) memcpy(&buffer[EMPTY_HEARTBEAT_MSG_SIZE],
                               this->data,
                               this->dataSize);
    return buffer;
}

void AppendEntriesRequest::serialPrint() {
#if PRINT_DEBUG
    Serial.printf("AppendEntries request message \n%-20s%-20s%-20s%-20s%-20s\n",
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
#endif // if PRINT_DEBUG
}

size_t AppendEntriesRequest::size() {
    return EMPTY_HEARTBEAT_MSG_SIZE + this->dataSize;
}

/**
 * --------------------------- AppendEntriesResponse ---------------------------
 */

AppendEntriesResponse::AppendEntriesResponse() {
    this->type = Message::AppendEntriesRes;
}

AppendEntriesResponse::AppendEntriesResponse(uint8_t *packet) {
    this->type       = Message::AppendEntriesRes;
    this->term       = unpack_uint32_t(packet, PACKET_BODY_OFFSET);
    this->success    = unpack_uint8_t(packet, PACKET_BODY_OFFSET + 4);
    this->matchIndex = unpack_uint16_t(packet, PACKET_BODY_OFFSET + 5);
    this->serverId   = unpack_uint32_t(packet, PACKET_BODY_OFFSET + 7);
}

uint8_t * AppendEntriesResponse::marshall() {
    uint8_t *buffer = new uint8_t[APP_ENTRIES_RES_MSG_SIZE];

    pack_uint8_t(buffer, 0, Message::AppendEntriesRes);
    pack_uint32_t(buffer, PACKET_BODY_OFFSET, this->term);
    pack_uint8_t(buffer, PACKET_BODY_OFFSET + 4, this->success);
    pack_uint16_t(buffer, PACKET_BODY_OFFSET + 5, this->matchIndex);
    pack_uint32_t(buffer, PACKET_BODY_OFFSET + 7, this->serverId);

    return buffer;
}

void AppendEntriesResponse::serialPrint() {
#if PRINT_DEBUG
    Serial.printf("AppendEntries response message \
                \n%-25s%-25s%-25s%-25s\n",
                  "term",
                  "success",
                  "matchIndex",
                  "senderId"
                  );

    Serial.printf("%-25lu%-25lu%-25lu%-25lu\n",
                  this->term,
                  this->success,
                  this->matchIndex,
                  this->serverId
                  );
#endif // if PRINT_DEBUG
}

size_t AppendEntriesResponse::size() {
    return APP_ENTRIES_RES_MSG_SIZE;
}

/**
 * --------------------         StateMachineRequest         --------------------
 */

StateMachineMessage::StateMachineMessage() {}

StateMachineMessage::StateMachineMessage(Message::_type msgType) {
    this->type     = msgType;
    this->data     = NULL;
    this->dataSize = 0;
}

StateMachineMessage::StateMachineMessage(uint8_t *packet, uint16_t size) {
    this->type     = (Message::_type)unpack_uint8_t(packet, 0);
    this->data     = packet + 1;
    this->dataSize = size - 1;
}

size_t StateMachineMessage::size() {
    return this->dataSize + 1;
}

uint8_t * StateMachineMessage::marshall() {
    // TODO: pack message type as well
    uint8_t *buffer = new uint8_t[this->dataSize + 1];

    pack_uint8_t(buffer, 0, this->type);

    if (this->dataSize) {
        memcpy(&buffer[1], this->data, this->dataSize);
        free(this->data);
        return buffer;
    }
    return NULL;
}

void StateMachineMessage::serialPrint() {
#if PRINT_DEBUG
    Serial.printf("State Machine Message type: %lu\n Size: %lu, Value: %lu\n",
                  this->type,
                  this->dataSize,
                  this->data[0]);
#endif // if PRINT_DEBUG
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

Message* e(uint8_t *packet, uint16_t size) {
    smMsg = StateMachineMessage(packet, size);
    return &smMsg;
}

/**
 * TODO: DOCS
 * NOTE: keep the order like we have it in Message::MessageType
 * [createMessage description]
 * @param  packet [description]
 * @return        [description]
 */
Message * (*messageGenerators[])(uint8_t * packet,
                                 uint16_t size) = { a, b, c, d };

Message* createMessage(uint8_t *packet, uint16_t size) {
    uint8_t messageType = unpack_uint8_t(packet, 0);

#if PRINT_DEBUG
    Serial.printf("MessageType: %d\n", messageType);
#endif // if PRINT_DEBUG

    if (messageType < Message::lastValForPeers) {
        return messageGenerators[messageType](packet, size);
    }

    if ((messageType > Message::firstValForClientReq) &&
        (messageType < Message::lastValForClientReq)) {
        return e(packet, size);
    }
    return NULL;
}
