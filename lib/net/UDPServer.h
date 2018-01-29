#ifndef rasp_udp_h
#define rasp_udp_h

#include "Arduino.h"
#include "WiFiUdp.h"
#include "messages.h"
#include "rasp_nodes.h"
#include "marshall.h"

// TODO: outsource constants
#define UDP_INCOMING_BUFFER_SIZE 2048 // TODO: SET TO MAX PACKET SIZE
#define UDP_PORT 1337

IPAddress sender;

// TODO: move function bodies into .cpp file
class UDPServer {
public:

    /**
     * TODO: DOCS
     * [start description]
     */
    void start() {
        Udp.begin(UDP_PORT);
        Serial.printf("UDP Server bind to port: %d\n", UDP_PORT);
    }

    /**
     * TODO: DOCS
     * [broadcastRequestVoteRPC description]
     * @param  message [description]
     * @return         [description]
     */
    void broadcastRequestVoteRPC(uint8_t *message) {
        RequestVoteRequest *req = new RequestVoteRequest(message);

        for (int i = 0; i < RASP_NUM_SERVERS; i++) {
            Udp.beginPacket(servers[i].IP, RASP_DEFAULT_PORT);
            Udp.write((char *)message, REQ_VOTE_REQ_MSG_SIZE);
            Udp.endPacket();
        }

        // free the message buffer after all messages are sent
        free(message);
    }

    /**
     * TODO: DOCS
     * [broadcastHeartbeat description]
     */
    void broadcastHeartbeat() {
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

    /**
     * TODO: DOCS
     * [checkForPacket description]
     */
    uint8_t* checkForIncomingPacket() {
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

            return packetBuffer;
        }
        return NULL;
    }

    /**
     * TODO: DOCS
     * [sendPacket description]
     * @param buffer [description]
     * @param size   [description]
     */
    void sendPacket(uint8_t *buffer, size_t size) {
        Serial.printf("Sending single message of size: %d to: %s\n",
                      size,
                      sender.toString().c_str());

        Udp.beginPacket(sender, RASP_DEFAULT_PORT);
        Udp.write((char *)buffer, size);
        Udp.endPacket();
    }

private:

    WiFiUDP Udp;
    uint8_t packetBuffer[UDP_INCOMING_BUFFER_SIZE];

    /**
     * TODO: DOCS
     * [clearBuffer description]
     */
    void clearBuffer() {
        memset(packetBuffer, 0, UDP_INCOMING_BUFFER_SIZE);
    }
};

#endif // ifndef rasp_udp_h
