#include "Log.h"


 #define TERM_OFFSET 0
 #define SIZE_OFFSET 4
 #define DATA_OFFSET 6 // term (4 bytes) + size (2 bytes)

/**
 * Returns the term number of a log entry
 * @param  ptr      pointer to a log entry
 * @return          term number of the log entry the pointer poits to
 */
uint32_t getTermNumber(uint8_t *ptr) {
    return unpack_uint32_t(ptr, TERM_OFFSET);
}

/**
 * Returns the size of the data of a log entry
 * @param  ptr      pointer to a log entry
 * @return          size of the log entry data the pointer poits to
 */
uint16_t getDataSize(uint8_t *ptr) {
    return unpack_uint16_t(ptr, SIZE_OFFSET);
}

/**
 * Returns true iff index is a valid index. That means a corresponding log entry
 * exist at that index.
 * @param  index    index to preform check on
 * @return       [description]
 */
bool validIndex(uint16_t index) {
    return (index > 0) && (index <= NUM_LOG_ENTRIES);
}

void Log::initialize() {
    nextEntry  = 0;
    latestTerm = 0;

    size_t currentLogSize = 0;
    size_t offset         = 0;

    currentLogSize = RASPFS::getInstance().readLog(this->data);

    Serial.printf("About to rebuild log from file system with size: %lu\n",
                  currentLogSize);

    // we iterate through all entries and initialies proper log values (i.e
    // lastTerm, nextEntry, entryAdress[])
    while (offset < currentLogSize) { // TODO: disable soft WDT
        Serial.printf("Latest term: %lu\nnextEntry: %lu\noffset: %lu\n",
                      latestTerm,
                      nextEntry,
                      offset);

        // set proper address of new entry
        entryAdress[nextEntry++] = offset;

        latestTerm = getTermNumber(&this->data[offset]);
        offset    += getDataSize(&this->data[offset]) + DATA_OFFSET;
    }
    Serial.printf("Latest term: %lu\nnextEntry: %lu\noffset: %lu\n",
                  latestTerm,
                  nextEntry,
                  offset);
}

uint16_t Log::append(uint32_t term, uint8_t *data, uint16_t size) {
    // only append if we did not reach the entires limit
    if (nextEntry == NUM_LOG_ENTRIES) {
        Serial.printf(
            "[ERR] Log entries length limit of %d reached, cannot append: %lub\n",
            NUM_LOG_ENTRIES,
            size);
        return 0;
    }

    uint16_t offset = this->size();

    // we dont store the next value if it would exceed our data buffer
    if (offset + size + DATA_OFFSET > LOG_SIZE) {
        Serial.printf(
            "[ERR] Log is full! Cannot append data.\n \
            New data size: %lub (+ 6 for term/size), used log size: %lub\n",
            size,
            offset);
        return 0;
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
    printLastEntry();
    return this->nextEntry;
}

void Log::truncate(uint16_t index) {
    if (!validIndex(index) || (lastIndex() < index)) return;

    this->nextEntry  = index;
    this->latestTerm = this->getTerm(index);

    RASPFS::getInstance().overwriteLog(this->data, this->size());
}

uint16_t Log::size() {
    if (lastIndex() == 0) return 0;

    uint16_t adr = lastEntryAddress();

    return adr + DATA_OFFSET + getDataSize(&this->data[adr]);
}

uint32_t Log::lastStoredTerm() {
    return this->latestTerm;
}

uint16_t Log::lastEntryAddress() {
    return !lastIndex() ? 0 : entryAdress[lastIndex() - 1];
}

logEntry_t * Log::getEntry(uint16_t index) {
    if (!validIndex(index) || (lastIndex() < index)) return NULL;

    logEntry_t *entry = new logEntry_t;

    uint8_t *p = getPointer(index);

    entry->term = getTermNumber(p);
    entry->size = getDataSize(p);
    entry->data = p + DATA_OFFSET;
    return entry;
}

uint32_t Log::getTerm(uint16_t index) {
    uint8_t *p = getPointer(index);

    return !index || !p ? 0 : getTermNumber(p);
}

uint16_t Log::lastIndex() {
    return this->nextEntry;
}

bool Log::exist(uint16_t index) {
    return validIndex(index) && index <= this->nextEntry;
}

void Log::printLastEntry() {
    uint8_t *p = this->getPointer(lastIndex());

    Serial.printf("LOG index: %lu\nTerm:%lu, Val: %lu\n",
                  lastIndex(),
                  getTermNumber(p),
                  unpack_uint8_t(p + DATA_OFFSET, 0));
}

uint8_t * Log::getPointer(uint16_t index) {
    return !validIndex(index) ? NULL : &this->data[this->entryAdress[index - 1]];
}
