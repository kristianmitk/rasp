#include "Log.h"


void Log::append(uint32_t term, uint8_t *data, uint16_t size) {
    // TODO: add missing variables
    uint16_t lastAddress = lastEntryAddress();
    uint16_t lastSize    = unpack_uint16_t(&this->data[lastAddress], 4);

    // +6 for term and size of data - check the
    uint16_t offset = lastAddress + 6 + lastSize;

    if (offset > LOG_SIZE) {
        Serial.printf(
            "[ERR] Log is full! Cannot append data.\n \
            New data size: %lu, used log size: %lu",
            size,
            offset);
        return;
    }

    if (nextEntry == 512) {
        Serial.printf("[ERR] Log entries size limit of %d reached",
                      NUM_LOG_ENTRIES);
        return;
    }

    uint8_t *p = &this->data[offset];

    // set term
    pack_uint32_t(p, 0, term);

    // set size
    pack_uint16_t(p, 4, size);

    // set data
    memcpy(p + 6, data, size);
    this->dataPointers[nextEntry] = offset;
    nextEntry++;

    latestTerm = max(latestTerm, term);

    RASPFS::getInstance().appendLogEntry(p, size + 6);
}

uint16_t Log::size() {
    return unpack_uint16_t(this->getPointer(lastIndex()),
                           4) + lastEntryAddress() + 6;
}

uint32_t Log::lastStoredTerm() {
    return this->latestTerm;
}

uint16_t Log::lastEntryAddress() {
    return dataPointers[lastIndex()];
}

uint16_t Log::lastIndex() {
    // cover initial 0 state
    return this->nextEntry ? this->nextEntry - 1 : this->nextEntry;
}

void Log::printLastEntry() {
    uint8_t *p = this->getPointer(lastIndex());

    Serial.printf("LOG index: %d\nTerm:%lu, Val: %d\n",
                  lastIndex(),
                  unpack_uint32_t(p, 0),
                  unpack_uint8_t(p + 6, 0));
}

uint8_t * Log::getPointer(uint16_t index) {
    return &this->data[this->dataPointers[index]];
}
