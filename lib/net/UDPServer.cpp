#include "UDPServer.h"


void UDPServer::start() {
    Udp.begin(UDP_PORT);
    Serial.printf("UDP Server bind to port: %d\n", UDP_PORT);
}

// TODO: use one broadcast Message function
void UDPServer::broadcastRequestVoteRPC(uint8_t *message) {
    for (int i = 0; i < RASP_NUM_SERVERS; i++) {
        if (servers[i].ID != chipID) {
            Udp.beginPacket(servers[i].IP, RASP_DEFAULT_PORT);
            Udp.write((char *)message, REQ_VOTE_REQ_MSG_SIZE);
            Udp.endPacket();
        }
    }
    free(message);
}

// TODO: use one broadcast Message function
void UDPServer::broadcastHeartbeat(uint8_t *message) {
    Serial.printf("Broadcasting heartbeat\n");

    for (int i = 0; i < RASP_NUM_SERVERS; i++) {
        if (servers[i].ID != chipID) {
            Serial.printf("Sending heartbeat to: %s\n", servers[i].IP);
            Udp.beginPacket(servers[i].IP, RASP_DEFAULT_PORT);
            Udp.write((char *)message, EMPTY_HEARTBEAT_MSG_SIZE);
            Udp.endPacket();
        }
    }
    free(message);
}

void UDPServer::sendPacket(uint8_t *buffer, size_t size) {
    Serial.printf("Sending single message of size: %d to: %s\n",
                  size,
                  sender.toString().c_str());

    Udp.beginPacket(sender, RASP_DEFAULT_PORT);
    Udp.write((char *)buffer, size);
    Udp.endPacket();
    free(buffer);
}

size_t UDPServer::parse() {
    this->currentPacketSize = Udp.parsePacket();

    if (currentPacketSize) {
        clearBuffer();

        // the IP is cached for 'one round' to respond after data is
        // processed
        sender = Udp.remoteIP();
#ifdef RASP_DEBUG
        Serial.printf(
            "--------------------------- %lu ---------------------------\n",
            eventNumber++
            );
        Serial.printf("Received %d bytes from %s\n",
                      currentPacketSize,
                      sender.toString().c_str());
#endif // ifdef RASP_DEBUG
        int len = Udp.read(packetBuffer, UDP_INCOMING_BUFFER_SIZE);
    }
    return currentPacketSize;
}

uint8_t * UDPServer::checkForPacket() {
    return this->parse() ? packetBuffer : NULL;
}

Message * UDPServer::checkForMessage() {
    return this->parse() ? createMessage(packetBuffer, currentPacketSize) : NULL;
}

void UDPServer::clearBuffer() {
    memset(packetBuffer, 0, UDP_INCOMING_BUFFER_SIZE);
}
