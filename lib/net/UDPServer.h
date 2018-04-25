#ifndef rasp_udp_h
#define rasp_udp_h

#include "Arduino.h"
#include "WiFiUdp.h"
#include "common.h"
#include "config.h"
#include "messages.h"
#include "rasp_nodes.h"
#include "marshall.h"
#include "ServerState.h"

typedef struct clientRequest {
    IPAddress ip;
    uint16_t  port;
    uint16_t  logIndex;
} clientRequest_t;

struct findClient {
    uint16_t logIndex;
    findClient(uint16_t logIndex) : logIndex(logIndex) {}

    bool operator()(const clientRequest& cr) const
    {
        return cr.logIndex == logIndex;
    }
};


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
    void start();

    /**
     * TODO: DOCS
     * [broadcastRequestVoteRPC description]
     * @param  message [description]
     * @return         [description]
     */
    void broadcastRequestVoteRPC(uint8_t *message);


    /**
     * TODO: DOCS
     * [checkForMessage description]
     * @return [description]
     */
    Message* checkForMessage();

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

    void sendPacket(uint8_t  *buffer,
                    size_t    size,
                    IPAddress ip,
                    uint16_t  port);

    /**
     * TODO: DOCS
     * @param logIndex [description]
     */
    void createClientRequest(uint16_t logIndex);

    std::vector<clientRequest_t>requests;

private:

    WiFiUDP Udp;
    uint8_t packetBuffer[UDP_INCOMING_BUFFER_SIZE];
    uint16_t currentPacketSize;

    /**
     * TODO: DOCS
     * [clearBuffer description]
     */
    void   clearBuffer();

    /**
     * TODO: DOCS
     * [parse description]
     * @return [description]
     */
    size_t parse();

    IPAddress senderIP;
    uint16_t senderPort;
    UDPServer(UDPServer const&);
    void operator=(UDPServer const&);

    /**
     * Constructor is private so we guarantee a singleton
     */
    UDPServer() {}
};


#endif // ifndef rasp_udp_h
