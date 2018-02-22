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

SerialInHandler serialInHandler;

/* -------------------------- SETUP -------------------------- */
void setup() {
    Serial.begin(DEFAULT_BAUD_RATE);

    // if analog input pin 0 is unconnected, random analog
    // noise will cause the call to randomSeed() to generate
    // different seed numbers each time the sketch runs.
    // randomSeed() will then shuffle the random function.
    randomSeed(analogRead(0));

    // lazy construction included
    ServerState::getInstance().initialize();

    // setup network connection
    connectToWiFi();

    // open server to listen for incomming packets
    UDPServer::getInstance().start();
}

/* -------------------------- LOOP -------------------------- */
void loop() {
    ServerState::getInstance().loopHandler();

    // TODO: add state-machine stuff

    serialInHandler.read();
}
