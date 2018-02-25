#ifndef common_h
#define common_h
#include <Arduino.h>

// TODO: add common stuff here and include where needed

// const uint32_t chipID = system_get_chip_id();
const uint32_t chipID = ESP.getChipId();

extern uint32_t eventNumber;

void printEventHeader(uint32_t term);

void printCurrentMillis();

#endif // ifndef common_h
