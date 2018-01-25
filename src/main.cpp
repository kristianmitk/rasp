#include <Arduino.h>
#include "rasp_wifi.h"
#include "messages.h"
#include "marshall.h"
#include "UDPServer.h"
#include "ServerState.h"
#include <fs.h>
extern "C" {
    #include "user_interface.h"
}

// #define INITIAL_SETUP
#define DEFAULT_BAUD_RATE 115200

UDPServer udpServer;
ServerState *currentState;
uint8_t     *incomingPacket;
RequestVoteRequest  reqVoteReqMsg;
RequestVoteResponse reqVoteResMsg;

/* -------------------------- SETUP -------------------------- */
void setup() {
    Serial.begin(DEFAULT_BAUD_RATE);
    uint32_t chipID = system_get_chip_id();
    Serial.printf("\nChip ID:%d\n", chipID);

#ifdef INITIAL_SETUP

    Serial.println("\n\n[INFO] Running initial setup");
    SPIFFS.remove("/SS/currentTerm");
    RASPFS::getInstance().write(CURRENT_TERM, 0);
    RASPFS::getInstance().write(VOTED_FOR, 0);

#endif // ifdef INITIAL_SETUP

    // setup network connection
    connectToWiFi();

    // open server to listen for incomming packets
    udpServer.start();

    // if analog input pin 0 is unconnected, random analog
    // noise will cause the call to randomSeed() to generate
    // different seed numbers each time the sketch runs.
    // randomSeed() will then shuffle the random function.
    randomSeed(analogRead(0));

    // TODO: do we need a Constructor?
    currentState = new ServerState(chipID);
}

/* -------------------------- LOOP -------------------------- */
void loop() {
    if (currentState->checkHeartbeatTimeout()) {
        // TODO: broadcast empty heartbeat
        udpServer.broadcastHeartbeat();
    }

    if ((reqVoteReqMsg = currentState->checkElectionTimeout())) {
        udpServer.broadcastRequestVoteRPC(reqVoteReqMsg.marshall());
    }

    if (incomingPacket = udpServer.checkForIncomingPacket()) {
        uint32_t type = unpack_uint32_t(incomingPacket, 0);

        // Serial.printf("type is : %lu\n", type);

        switch (type) {
        case RequestVoteReq:

            reqVoteReqMsg = RequestVoteRequest(incomingPacket);
            udpServer.sendPacket(currentState->handleRequestVoteReq(
                                     reqVoteReqMsg).marshall(),
                                 REQ_VOTE_RES_MSG_SIZE);

            break;

        case RequestVoteRes:

            reqVoteResMsg = RequestVoteResponse(incomingPacket);
            currentState->handleRequestVoteRes(reqVoteResMsg);


            break;

        case AppendEntriesReq:
            Serial.println("<3");
            currentState->resetElectionTimeout(1);

            break;

        case AppendEntriesRes:

            // TODO: implement
            break;
        }
    } else {
        // TODO: handle stuff when no packet is incoming:
        //  - check for timeouts and resend packets
        //  - appendEntriesRPC
    }
}
