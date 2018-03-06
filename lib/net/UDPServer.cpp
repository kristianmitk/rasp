#include "UDPServer.h"


void UDPServer::start() {
    Udp.begin(RASP_DEFAULT_PORT);
    Serial.printf("UDP Server bind to port: %d\n", RASP_DEFAULT_PORT);
}

// TODO: use followerState IPs
void UDPServer::broadcastRequestVoteRPC(uint8_t *message) {
    for (int i = 0; i < RASP_NUM_SERVERS; i++) {
        if (servers[i].ID != chipId) {
            Udp.beginPacket(servers[i].IP, RASP_DEFAULT_PORT);
            Udp.write((char *)message, REQ_VOTE_REQ_MSG_SIZE);
            Udp.endPacket();
        }
    }
    free(message);
}

void UDPServer::sendPacket(uint8_t *buffer, size_t size) {
    Serial.printf("Sending single message of size: %d to: %s\n",
                  size,
                  senderIP.toString().c_str());

    Udp.beginPacket(senderIP, senderPort);
    Udp.write((char *)buffer, size);
    Udp.endPacket();
    free(buffer);
}

void UDPServer::sendPacket(uint8_t *buffer, size_t size, uint8_t IP[4]) {
    IPAddress addr(IP);

    Serial.printf("Sending single message of size: %d to: ",
                  size);
    Serial.println(addr);
    Udp.beginPacket(addr, RASP_DEFAULT_PORT);
    Udp.write((char *)buffer, size);
    Udp.endPacket();
    free(buffer);
}

void UDPServer::sendPacket(uint8_t  *buffer,
                           size_t    size,
                           IPAddress ip,
                           uint16_t  port) {
    Serial.printf("Sending single message of size: %d to: ",
                  size);
    Serial.println(ip);
    Serial.printf(":%d\n", port);
    Udp.beginPacket(ip, port);
    Udp.write((char *)buffer, size);
    Udp.endPacket();
    free(buffer);
}

size_t UDPServer::parse() {
    this->currentPacketSize = Udp.parsePacket();

    if (currentPacketSize) {
        clearBuffer();

        this->senderIP   = Udp.remoteIP();
        this->senderPort = Udp.remotePort();
        printEventHeader(ServerState::getInstance().getCurrentTerm());
        Serial.printf("Received %d bytes from %s\n",
                      currentPacketSize,
                      senderIP.toString().c_str());

        Udp.read(packetBuffer, UDP_INCOMING_BUFFER_SIZE);
    }
    return currentPacketSize;
}

Message * UDPServer::checkForMessage() {
    return this->parse() ? createMessage(packetBuffer, currentPacketSize) : NULL;
}

void UDPServer::clearBuffer() {
    memset(packetBuffer, 0, UDP_INCOMING_BUFFER_SIZE);
}

// TODO: add request ID
void UDPServer::createClientRequest(uint16_t logIndex) {
    clientRequest_t req;

    req.ip       = senderIP;
    req.port     = senderPort;
    req.logIndex = logIndex;
    this->requests.push_back(req);
    Serial.printf(
        "Added a new client request for index: %lu\nNum requests: %lu\n",
        senderPort,
        logIndex,
        this->requests.size());
}
