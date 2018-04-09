#ifndef common_h
#define common_h
#include <Arduino.h>

// TODO: add common stuff here and include where needed

// used as the ID of a node
const uint32_t chipId = ESP.getChipId();

// used for logging
extern uint32_t eventNumber;

/**
 * Prints a header with informations (event number, current term, current
 * millis). Used for better logging
 * @param term      current Term stored by the ServerState singleton
 */
void printEventHeader(uint32_t term);

#endif // ifndef common_h
