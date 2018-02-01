#include "Log.h"


void Log::append(logEntry_t newEntry) {
    if (nextEntry == LOG_LENGTH) {
        Serial.printf("[ERR] LOG IS FULL! Appending entry not possible");
        return;
    }
    entries[nextEntry] = newEntry;
    nextEntry++;
    RASPFS::getInstance().appendLogEntry(newEntry);
}

size_t Log::size() {
    return this->nextEntry;
}

uint32_t Log::lastStoredTerm() {
    return this->latestTerm;
}

logEntry_t Log::lastEntry() {
    uint8_t index = this->nextEntry;

    return read(index == 0 ? 0 : index - 1);
}

uint8_t Log::lastIndex() {
    return this->nextEntry ? this->nextEntry - 1 : this->nextEntry;
}

logEntry_t Log::read(uint32_t index) {
    return entries[abs(index) % LOG_LENGTH];
}

void Log::printLastEntry() {
    logEntry_t last = lastEntry();

    Serial.printf("LOG index: %d\nTerm:%lu, Val: %d\n",
                  this->nextEntry,
                  last.term,
                  unpack_uint8_t(last.data, 0));
}
