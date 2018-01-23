#include <Arduino.h>
#include "rasp_wifi.h"
#include "rasp_udp.h"
#include "ServerState.h"
#include "messages.h"
#include "util.h"
extern "C" {
// espressif ESP8266 sdk
    #include "user_interface.h"

// #include "stdlib.h"
}

#define DEFAULT_BAUD_RATE 115200

UDP_Server   udpServer;
ServerState *currentState;
uint8_t     *incomingPacket;
RequestVoteRequest  reqVoteReqMsg;
RequestVoteResponse reqVoteResMsg;


/* -------------------------- SETUP -------------------------- */
void setup() {
    Serial.begin(DEFAULT_BAUD_RATE);

    Serial.printf("\nChip-ID: %d\n", system_get_chip_id());

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
    currentState = new ServerState(system_get_chip_id());
}

/* -------------------------- LOOP -------------------------- */
void loop() {
    if ((currentState->getRole() == LEADER) &&
        currentState->checkHeartbeatTimeout()) {
        udpServer.broadcastHeartbeat();
    }

    if ((currentState->getRole() != LEADER) &&
        (reqVoteReqMsg = currentState->checkElectionTimeout())) {
        udpServer.broadcastRequestVoteRPC(reqVoteReqMsg.marshall());
    }

    if (incomingPacket = udpServer.checkForIncomingPacket()) {
        // TODO: little endian or big endian?
        uint32_t type = unpack_uint32_t(incomingPacket, 0);

        Serial.printf("type is : %lu\n", type);

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
    }
}
