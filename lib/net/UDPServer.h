#ifndef rasp_udp_h
#define rasp_udp_h

#include "Arduino.h"
#include "WiFiUdp.h"
#include "common.h"
#include "env.h"
#include "messages.h"
#include "rasp_nodes.h"
#include "marshall.h"

// TODO: outsource constants

// 512b for entries data + 17 for the other stuff needed in a AppendEntries req
#define UDP_INCOMING_BUFFER_SIZE 529

// default port
#define RASP_DEFAULT_PORT 1337

/**
 * Singleton class that handles in/out messaging between peers
 * TODO: DOCS
 * [getInstance description]
 * @return [description]
 */
class UDPServer {
public:

    static UDPServer& getInstance()
    {
        static UDPServer instance;

        return instance;
    }

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

    /**
     * TODO: DOCS
     * [sendPacket description]
     * @param buffer [description]
     * @param size   [description]
     * @param IP     [description]
     */
    void sendPacket(uint8_t *buffer,
                    size_t   size,
                    uint8_t  IP[4]);

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

    UDPServer(UDPServer const&);
    void operator=(UDPServer const&);

    /**
     * Constructor is private so we guarantee a singleton
     */
    UDPServer() {}
};


#endif // ifndef rasp_udp_h
