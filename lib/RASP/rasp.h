#include <Arduino.h>

#include "config.h"
#include "common.h"
#include "rasp_wifi.h"
#include "messages.h"
#include "marshall.h"
#include "UDPServer.h"
#include "ServerState.h"
#include "StateMachine.h"

/* -------------------------- SETUP -------------------------- */
void RASP_SETUP() {
    Serial.begin(DEFAULT_BAUD_RATE);
    Serial.println();

    // if analog input pin 0 is unconnected, random analog
    // noise will cause the call to randomSeed() to generate
    // different seed numbers each time the sketch runs.
    // randomSeed() will then shuffle the random function.
    randomSeed(analogRead(0));

    // setup network connection
    connectToWiFi();

    // lazy construction and state initialization
    ServerState::getInstance().initialize();

    StateMachine::getInstance();

    // lazy construction and and open server
    // NOTE: keep this always as last command in this setup() function
    // to avoid having old messages in the incoming buffer
    UDPServer::getInstance().start();
}

/* -------------------------- LOOP -------------------------- */
void RASP_LOOP() {
    ServerState::getInstance().loopHandler();
}
