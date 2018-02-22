#ifndef common_h
#define common_h
#include <Arduino.h>
extern "C" {
    #include "user_interface.h"
}

// TODO: add common stuff here and include where needed

const uint32_t chipID = system_get_chip_id();

extern uint32_t eventNumber;

void printEventHeader();

#endif // ifndef common_h
