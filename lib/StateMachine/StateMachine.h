#ifndef StateMachine_h
#define StateMachine_h

#include "SM_Interface.h"

#define LED_PIN 5

/**
 * Simple binary state machine as an example/placeholder to debug
 */

class StateMachine : SM_Interface {
public:

    StateMachine() {
        this->state = 0;
        pinMode(LED_PIN, OUTPUT);
        digitalWrite(LED_PIN, this->state);
    }

    smData_t* apply(const void *data, size_t dataSize) {
        this->state = ((uint8_t *)data)[0] % 2;
        digitalWrite(LED_BUILTIN, this->state);
        Serial.printf("Applied to SM new state: %d\n", this->state);

        return this->createResponse();
    }

    /**
     * This function serves for read requests on the state machine.
     * Depending on the passed data it will return whatever was requested.
     * It is hold as generic as possible
     * @param  data         data needed in order to indentify request
     * @param  dataSize     size of datablock
     * @return {smData_t}   struct holding state machine data
     */
    smData_t* read(const void *data, size_t dataSize) {
        return this->createResponse();
    }

private:

    uint8_t state;

    smData_t* createResponse() {
        size_t   stateSize = sizeof(this->state);
        uint8_t *buffer    = new uint8_t[stateSize];

        buffer[0]  = this->state;
        smRes.data = buffer;
        smRes.size = stateSize;

        return &smRes;
    }
};

#endif // ifndef StateMachine_h
