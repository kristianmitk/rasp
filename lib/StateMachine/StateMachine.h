#ifndef StateMachine_h
#define StateMachine_h

#include <Arduino.h>
#include "SM_Interface.h"
#include "marshall.h"

#define LED_PIN 5

/**
 * Simple binary state machine as an example/placeholder to debug
 */
class StateMachine : SM_Interface {
public:

    static StateMachine& getInstance() {
        static StateMachine instance;

        return instance;
    }

    void apply(const void *data) {
        this->state = ((uint8_t *)data)[0];
        Serial.printf("Applied in SM new state: %d", this->state);

        if (this->state) {
            digitalWrite(LED_BUILTIN, HIGH);
        } else {
            digitalWrite(LED_BUILTIN, LOW);
        }
    }

    /**
     * This function serves for read requests on the state machine.
     * Depending on the passed data it will return whatever was requested.
     * It is hold as generic as possible
     * @param  data data needed in order to indentify request
     * @return      byte buffer holding requested data
     */
    uint8_t* read(const void *data) {
        uint8_t *buffer = new uint8_t[1];

        buffer[0] = this->state;
        return buffer;
    }

private:

    uint8_t state;
    StateMachine() {
        this->state = 0;
        pinMode(LED_PIN, OUTPUT);
        digitalWrite(LED_PIN, LOW);
    }

    StateMachine(StateMachine const&);
    void operator=(StateMachine const&);
};

#endif // ifndef StateMachine_h
