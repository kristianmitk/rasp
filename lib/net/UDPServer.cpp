#include "UDPServer.h"


void UDPServer::start() {
    Udp.begin(RASP_DEFAULT_PORT);
    Serial.printf("UDP Server bind to port: %d\n", RASP_DEFAULT_PORT);
}

void UDPServer::broadcastRequestVoteRPC(uint8_t *message) {
#ifdef USE_BROADCAST_ADDRESS
    Udp.beginPacket("192.168.1.255", RASP_DEFAULT_PORT);
    Udp.write((char *)message, REQ_VOTE_REQ_MSG_SIZE);
    Udp.endPacket();
#else
    for (int i = 0; i < RASP_NUM_SERVERS; i++) {
        if (servers[i].ID != chipId) {
            Udp.beginPacket(servers[i].IP, RASP_DEFAULT_PORT);
            Udp.write((char *)message, REQ_VOTE_REQ_MSG_SIZE);
            Udp.endPacket();
        }
    };
#endif // ifndef USE_BROADCAST_ADDRESS
    free(message);
}

void UDPServer::sendPacket(uint8_t *buffer, size_t size) {
    RASPDBG("Sending single message of size: %d to: %s\n",
            size,
            senderIP.toString().c_str())

    Udp.beginPacket(senderIP, senderPort);
    Udp.write((char *)buffer, size);
    Udp.endPacket();
    free(buffer);
}

void UDPServer::sendPacket(uint8_t *buffer, size_t size, uint8_t IP[4]) {
    IPAddress addr(IP);

#if PRINT_DEBUG
    Serial.printf("Sending single message of size: %d to: ", size);
    Serial.println(addr);
#endif // if PRINT_DEBUG
    Udp.beginPacket(addr, RASP_DEFAULT_PORT);
    Udp.write((char *)buffer, size);
    Udp.endPacket();
    free(buffer);
}

void UDPServer::sendPacket(uint8_t  *buffer,
                           size_t    size,
                           IPAddress ip,
                           uint16_t  port) {
#if PRINT_DEBUG
    Serial.printf("Sending single message of size: %d to: ", size);
    Serial.print(ip);
    Serial.printf(":%d\n", port);
#endif // if PRINT_DEBUG

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
        RASPDBG("Received %d bytes from %s\n",
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
    if (!logIndex) {
        smMsg.type     = Message::StateMachineWriteERR;
        smMsg.dataSize = 0;
        sendPacket(smMsg.marshall(), smMsg.size(), senderIP, senderPort);
    } else {
        clientRequest_t req;

        req.ip       = senderIP;
        req.port     = senderPort;
        req.logIndex = logIndex;
        this->requests.push_back(req);
        RASPDBG("Added a new client request for index: %lu\nNum requests: %lu\n",
                senderPort,
                logIndex,
                this->requests.size())
    }
}
