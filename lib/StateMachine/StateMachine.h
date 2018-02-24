#ifndef StateMachine_h
#define StateMachine_h

#include <Arduino.h>


#define LED_PIN 5

class StateMachine {
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
