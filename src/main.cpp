#include <Arduino.h>

#include "rasp_wifi.h"
#include "messages.h"
#include "marshall.h"
#include "UDPServer.h"
#include "ServerState.h"
#include "SerialInHandler.h"

extern "C" {
    #include "user_interface.h"
}

#define INITIAL_SETUP
#define DEFAULT_BAUD_RATE 115200

UDPServer udpServer;
SerialInHandler serialInHandler;
ServerState    *currentState;
Message *msg;
RequestVoteRequest reqVoteReqMsg;

/* -------------------------- SETUP -------------------------- */
void setup() {
    Serial.begin(DEFAULT_BAUD_RATE);
    uint32_t chipID = system_get_chip_id();
    Serial.printf("\nChip ID:%d\n", chipID);

#ifdef INITIAL_SETUP

    Serial.println("\n\n[INFO] Running initial setup");
    RASPFS::getInstance().write(RASPFS::CURRENT_TERM, 0);
    RASPFS::getInstance().write(RASPFS::VOTED_FOR, 0);
    RASPFS::getInstance().remove(RASPFS::LOG);

#endif // ifdef INITIAL_SETUP

    // setup network connection
    connectToWiFi();

    // if analog input pin 0 is unconnected, random analog
    // noise will cause the call to randomSeed() to generate
    // different seed numbers each time the sketch runs.
    // randomSeed() will then shuffle the random function.
    randomSeed(analogRead(0));

    // TODO: do we need a Constructor?
    currentState = new ServerState(chipID);

    // open server to listen for incomming packets
    udpServer.start();
}

/* -------------------------- LOOP -------------------------- */
uint32_t numLoops = 0;
void loop() {
    ++numLoops;
    msg = NULL;
    currentState->DEBUG_APPEND_LOG();

    if (currentState->checkHeartbeatTimeout()) {
        Serial.printf("\n------------------ %lu ------------------\n", numLoops);
        udpServer.broadcastHeartbeat();
    }

    if ((msg = currentState->checkElectionTimeout())) {
        Serial.printf("\n------------------ %lu ------------------\n", numLoops);
        udpServer.broadcastRequestVoteRPC(msg->marshall());
    }

    if (msg = udpServer.checkForMessage()) {
        Serial.printf("\n------------------ %lu ------------------\n", numLoops);
        Message *res = currentState->dispatch(msg);

        if (res) {
            udpServer.sendPacket(res->marshall(), REQ_VOTE_RES_MSG_SIZE);
        }

        return;
    }

    // TODO: handle stuff when no packet is incoming:
    //  - check for timeouts and resend packets
    //  - appendEntriesRPC
    serialInHandler.read();
}
