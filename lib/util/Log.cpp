#include "Log.h"


void Log::append(LogEntry newEntry) {
    if (nextEntry == LOG_SIZE) {
        Serial.printf("[ERR] LOG IS FULL! Appending entry not possible");
        return;
    }
    entries[nextEntry] = newEntry;
    nextEntry++;

    // TODO: append serialized logEntry
    // RASPFS::getInstance().appendString(LOG, (char *)newEntry.command);
}

size_t Log::size() {
    return this->nextEntry;
}

uint32_t Log::lastStoredTerm() {
    return this->latestTerm;
}

LogEntry Log::read(uint32_t index) {
    return entries[abs(index) % LOG_SIZE];
}
