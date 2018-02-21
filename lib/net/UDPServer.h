#ifndef rasp_udp_h
#define rasp_udp_h

#include "Arduino.h"
#include "WiFiUdp.h"
#include "common.h"
#include "messages.h"
#include "rasp_nodes.h"
#include "marshall.h"

// TODO: outsource constants
#define UDP_INCOMING_BUFFER_SIZE 529 // 512b for entries data + 17 for the other
                                     // stuff needed in a AppendEntries req
#define UDP_PORT 1337

class UDPServer {
public:

    /**
     * TODO: DOCS
     * [start description]
     */
    void     start();

    /**
     * TODO: DOCS
     * [broadcastRequestVoteRPC description]
     * @param  message [description]
     * @return         [description]
     */
    void     broadcastRequestVoteRPC(uint8_t *message);

    /**
     * TODO: DOCS
     * [broadcastHeartbeat description]
     */
    void     broadcastHeartbeat(uint8_t *message);

    /**
     * TODO: DOCS
     * [checkForPacket description]
     */
    uint8_t* checkForPacket();

    /**
     * TODO: DOCS
     * [checkForMessage description]
     * @return [description]
     */
    Message* checkForMessage();

    /**
     * TODO: DOCS
     * [parse description]
     * @return [description]
     */
    size_t   parse();

    /**
     * TODO: DOCS
     * [sendPacket description]
     * @param buffer [description]
     * @param size   [description]
     */
    void     sendPacket(uint8_t *buffer,
                        size_t   size);

private:

    WiFiUDP Udp;
    uint8_t packetBuffer[UDP_INCOMING_BUFFER_SIZE];
    uint16_t currentPacketSize;

    /**
     * TODO: DOCS
     * [clearBuffer description]
     */
    void clearBuffer();

    IPAddress sender;
};


#endif // ifndef rasp_udp_h
