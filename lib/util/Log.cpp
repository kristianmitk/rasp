#include "Log.h"

/**
 * In the following the structure of a log entry is visualized:
 *
 * +----------------------------------------+
 * |        |       |                       |
 * -- term -- size -- data ------------------
 * 0        4       6                       x = size
 * |        |       |                       |
 * +----------------------------------------+
 *
 * -> Log entries may have dynamic sizes.
 */
 #define TERM_OFFSET 0
 #define SIZE_OFFSET 4
 #define DATA_OFFSET 6 // term (4 bytes) + size (2 bytes)

/**
 * Returns the term number of a log entry
 * @param  ptr pointer to a log entry
 * @return     term number of the log entry the pointer poits to
 */
uint32_t getTermNumber(uint8_t *ptr) {
    return unpack_uint32_t(ptr, 0);
}

/**
 * Returns the size of the data of a log entry
 * @param  ptr pointer to a log entry
 * @return     size of the log entry data the pointer poits to
 */
uint16_t getDataSize(uint8_t *ptr) {
    return unpack_uint16_t(ptr, SIZE_OFFSET);
}

Log::Log() {
    nextEntry  = 0;
    latestTerm = 0;

    size_t currentLogSize = 0;
    size_t offset         = 0;

    currentLogSize = RASPFS::getInstance().readLog(this->data);

#ifdef RASP_DEBUG
    Serial.printf("About to rebuild log from file system with size: %lu\n",
                  currentLogSize);
#endif // ifdef RASP_DEBUG

    // we iterate through all entries and initialies proper log values (i.e
    // lastTerm, nextEntry, entryAdress[])
    while (offset < currentLogSize) {
#ifdef RASP_DEBUG
        Serial.printf("Latest term: %lu\nnextEntry: %lu\noffset: %lu\n",
                      latestTerm,
                      nextEntry,
                      offset);
#endif // ifdef RASP_DEBUG
        // set proper address of new entry
        entryAdress[nextEntry++] = offset;

        latestTerm = getTermNumber(&this->data[offset]);
        offset    += getDataSize(&this->data[offset]) +
                     DATA_OFFSET;
    }
#ifdef RASP_DEBUG
    Serial.printf("Latest term: %lu\nnextEntry: %lu\noffset: %lu\n",
                  latestTerm,
                  nextEntry,
                  offset);
#endif // ifdef RASP_DEBUG
}

void Log::append(uint32_t term, uint8_t *data, uint16_t size) {
    // only append if we did not reach the entires limit
    if (nextEntry == 512) {
        Serial.printf(
            "[ERR] Log entries size limit of %db reached, cannot append: %lub",
            NUM_LOG_ENTRIES,
            size);
        return;
    }

    uint16_t offset = this->size();

    // we dont store the next value if it would exceed our data buffer
    if (this->size() + size + DATA_OFFSET > LOG_SIZE) {
        Serial.printf(
            "[ERR] Log is full! Cannot append data.\n \
            New data size: %lub (+ 6 for term/size), used log size: %lub",
            size,
            offset);
        return;
    }

    uint8_t *p = &this->data[offset];

    // set term
    pack_uint32_t(p, TERM_OFFSET, term);

    // set size
    pack_uint16_t(p, SIZE_OFFSET, size);

    // set data
    memcpy(p + DATA_OFFSET, data, size);
    this->entryAdress[nextEntry++] = offset;

    latestTerm = term;

    RASPFS::getInstance().appendLogEntry(p, size + DATA_OFFSET);
}

uint16_t Log::size() {
    uint16_t adr = lastEntryAddress();

    return getDataSize(&this->data[adr]) + adr + DATA_OFFSET;
}

uint32_t Log::lastStoredTerm() {
    return this->latestTerm;
}

uint16_t Log::lastEntryAddress() {
    return entryAdress[lastIndex()];
}

logEntry_t Log::getEntry(uint16_t index) {
    logEntry_t entry;

    // empty size suggests that this entry does not exist
    entry.size = 0;

    // handle initial case where log is empty
    // TODO: rather use a pointer, but whats with memory fragmentation?
    if (this->nextEntry == 0) {
        entry.size = 1;
        entry.term = 0;
        return entry;
    }

    if (lastIndex() >= index) {
        uint8_t *p = getPointer(index);

        entry.term = getTermNumber(p);
        entry.size = getDataSize(p);
        entry.data = p + DATA_OFFSET;
    }

    return entry;
}

uint16_t Log::lastIndex() {
    // cover initial 0 state
    return this->nextEntry ? this->nextEntry - 1 : this->nextEntry;
}

void Log::printLastEntry() {
    uint8_t *p = this->getPointer(lastIndex());

    Serial.printf("LOG index: %lu\nTerm:%lu, Val: %lu\n",
                  lastIndex(),
                  getTermNumber(p),
                  unpack_uint8_t(p + DATA_OFFSET, 0));
}

uint8_t * Log::getPointer(uint16_t index) {
    return &this->data[this->entryAdress[index]];
}
