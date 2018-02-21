#include <Arduino.h>

#include "env.h"
#include "common.h"
#include "rasp_wifi.h"
#include "messages.h"
#include "marshall.h"
#include "UDPServer.h"
#include "ServerState.h"
#include "SerialInHandler.h"
extern "C" {
    #include "user_interface.h"
}

#define DEFAULT_BAUD_RATE 115200

UDPServer udpServer;
SerialInHandler serialInHandler;
ServerState     currentState;
Message *msg;

/* -------------------------- SETUP -------------------------- */
void setup() {
    Serial.begin(DEFAULT_BAUD_RATE);

    // if analog input pin 0 is unconnected, random analog
    // noise will cause the call to randomSeed() to generate
    // different seed numbers each time the sketch runs.
    // randomSeed() will then shuffle the random function.
    randomSeed(analogRead(0));

    currentState.initialize();

    // setup network connection
    connectToWiFi();

    // open server to listen for incomming packets
    udpServer.start();
}

/* -------------------------- LOOP -------------------------- */
uint32_t numLoops = 0;
void loop() {
    ++numLoops;
    msg = NULL;
    currentState.DEBUG_APPEND_LOG();

    if (currentState.checkHeartbeatTimeout()) {
        udpServer.broadcastHeartbeat();
        Serial.printf("\n------------------ %lu ------------------\n", numLoops);
    }

    if ((msg = currentState.checkElectionTimeout())) {
        udpServer.broadcastRequestVoteRPC(msg->marshall());
        Serial.printf("\n------------------ %lu ------------------\n", numLoops);
    }

    if (msg = udpServer.checkForMessage()) {
        Message *res = currentState.dispatch(msg);

        if (res) {
            udpServer.sendPacket(res->marshall(), REQ_VOTE_RES_MSG_SIZE);
        }
        Serial.printf("\n------------------ %lu ------------------\n", numLoops);
        return;
    }

    // TODO: handle stuff when no packet is incoming:
    //  - check for timeouts and resend packets
    //  - appendEntriesRPC
    serialInHandler.read();
}
