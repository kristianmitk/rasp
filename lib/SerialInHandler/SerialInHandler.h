#ifndef Serial_In_Handler_H
#define Serial_In_Handler_H
#include <Arduino.h>

class SerialInHandler {
public:

    SerialInHandler() {
        incomingByte = 0;
        available    = 0;
        command      = "";
    }

    void read() {
        if ((Serial.available()) > 0) {
            incomingByte = Serial.read();

            if (incomingByte == '\n') {
                parseInput();
            } else {
                if (incomingByte != '\r') command += incomingByte;
            }
        }
    }

private:

    void parseInput() {
        Serial.println("[INFO] Executing serial command: \"" + command + "\"");

        if (command == "millis") {
            Serial.printf("Current millis: %lu", millis());
        } else if (command == "micros") {
            Serial.printf("Current micros: %lu", micros());
        }

        command = "";
    }

    char incomingByte; // for incoming serial data
    int available;     // check for available bytes
    String command;
};
#endif // ifndef Serial_In_Handler_H
