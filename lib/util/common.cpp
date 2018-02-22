#include "common.h"

uint32_t eventNumber = 0;

void printEventHeader() {
    Serial.printf(
        "--------------------------- %lu ---------------------------\n",
        eventNumber++
        );
}
