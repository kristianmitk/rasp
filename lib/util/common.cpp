#include "common.h"

uint32_t eventNumber = 0;

void printEventHeader(uint32_t term) {
    Serial.printf(
        "\n------------------ Event: %lu Term: %lu Millis: %lu ------------------\n",
        eventNumber++,
        term,
        millis()
        );
}

void printCurrentMillis() {
    Serial.printf("current millis: %lu\n", millis());
}
