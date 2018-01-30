#include "UDPServer.h"

void UDPServer::start() {
    Udp.begin(UDP_PORT);
    Serial.printf("UDP Server bind to port: %d\n", UDP_PORT);
}

void UDPServer::broadcastRequestVoteRPC(uint8_t *message) {
    for (int i = 0; i < RASP_NUM_SERVERS; i++) {
        Udp.beginPacket(servers[i].IP, RASP_DEFAULT_PORT);
        Udp.write((char *)message, REQ_VOTE_REQ_MSG_SIZE);
        Udp.endPacket();
    }
    free(message);
}

void UDPServer::broadcastHeartbeat() {
    Serial.printf("Broadcasting heartbeat\n");

    uint8_t buf[4];
    pack_uint32_t(buf, 0, 201);

    for (int i = 0; i < RASP_NUM_SERVERS; i++) {
        Serial.printf("Sending heartbeat to: %s\n", servers[i].IP);
        Udp.beginPacket(servers[i].IP, RASP_DEFAULT_PORT);
        Udp.write((char *)buf, 4);
        Udp.endPacket();
    }
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
    int packetSize = Udp.parsePacket();

    if (packetSize) {
        clearBuffer();

        // the IP is cached for 'one round' to respond after data is
        // processed
        sender = Udp.remoteIP();
        Serial.printf("Received %d bytes from %s\n",
                      packetSize,
                      sender.toString().c_str());

        int len = Udp.read(packetBuffer, UDP_INCOMING_BUFFER_SIZE);
    }
    return packetSize;
}

uint8_t * UDPServer::checkForPacket() {
    return this->parse() ? packetBuffer : NULL;
}

Message * UDPServer::checkForMessage() {
    return this->parse() ? createMessage(packetBuffer) : NULL;
}

void UDPServer::clearBuffer() {
    memset(packetBuffer, 0, UDP_INCOMING_BUFFER_SIZE);
}
