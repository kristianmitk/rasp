#include <Arduino.h>

#include "Log.h"

extern "C" {
    #include "user_interface.h"
}

#define DEFAULT_BAUD_RATE 115200

#ifndef RASP_DBG

# define RASP_DBG(...) Serial.printf(__VA_ARGS__)

#endif // ifndef RASP_DBG
Log *_log;

/* -------------------------- SETUP -------------------------- */
void setup() {
    Serial.begin(DEFAULT_BAUD_RATE);
    Serial.println();

    // lazy init
    RASPFS::getInstance();
    _log = new Log();
}

/* -------------------------- LOOP -------------------------- */
int loopNum = 0;
void loop() {
    loopNum++;
    RASP_DBG("Loop num: %lu\n", loopNum);

    if (loopNum % 5 == 0) {
        _log->truncate(2);
    }
    uint8_t buf[48];
    _log->append(loopNum, buf, 48);
    delay(1000);
}
