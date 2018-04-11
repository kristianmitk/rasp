#include "common.h"
#include "config.h"
uint32_t eventNumber = 0;

void printEventHeader(uint32_t term) {
    RASPDBG(
        "\n--------------- Event: %lu Term: %lu Millis: %lu ---------------\n",
        eventNumber++,
        term,
        millis()
        )
}
