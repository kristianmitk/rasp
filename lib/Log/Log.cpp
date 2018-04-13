#include "Log.h"


#define TERM_OFFSET 0
#define DATA_OFFSET 4

#define RASPFS RASPFS::getInstance()


/**
 * Returns the term number of a log entry
 * @param  ptr      pointer to a log entry
 * @return          term number of the log entry the pointer poits to
 */
uint32_t getTermNumber(uint8_t *ptr) {
    if (!ptr) return 0;

    return unpack_uint32_t(ptr, TERM_OFFSET);
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
    size_t adrSize     = RASPFS.readLogAddresses((uint8_t *)&this->entryAddress);
    size_t logDataSize = RASPFS.readLogData(this->data);

    // adrSize is always even numbered as we store 2 byte integers there
    // so the number of entries is adrSize / 2
    nextEntry = adrSize / 2;

    // so we know where to store the next entry and where the last one ends
    if (nextEntry <= NUM_LOG_ENTRIES) this->entryAddress[nextEntry] = logDataSize;

    latestTerm = nextEntry ?
                 getTermNumber(&this->data[this->entryAddress[nextEntry - 1]])
                 : 0;


    Serial.printf("[INFO] Rebuilt log from file-system!\n"
                  "number of entries: %d, latestTerm: %d, data size: %d\n",
                  nextEntry,
                  latestTerm,
                  logDataSize);
}

uint16_t Log::append(uint32_t term, uint8_t *data, uint16_t size) {
    // only append if we did not reach the entires limit
    if (nextEntry == NUM_LOG_ENTRIES) {
        Serial.printf("[ERR] Log entries length limit of %d reached!\n",
                      NUM_LOG_ENTRIES,
                      size);
        return 0;
    }

    uint16_t offset = this->size();

    // we dont store the next value if it would exceed our data buffer
    if (offset + size + DATA_OFFSET > LOG_SIZE) {
        Serial.printf(
            "[ERR] Not enought memory space to append new entry!\n"
            "New entry size: %lu, used size: %lu\n"
            "max size is: %lu",
            size,
            offset,
            LOG_SIZE);
        return 0;
    }

    uint8_t *p = &this->data[offset];

    // set term
    pack_uint32_t(p, TERM_OFFSET, term);
    memcpy(p + DATA_OFFSET, data, size);

    // note: len(entryAddress) == NUM_LOG_ENTRIES + 1
    if ((++nextEntry) <= NUM_LOG_ENTRIES) {
        this->entryAddress[nextEntry] = offset + DATA_OFFSET + size;
    }

    RASPFS.appendLogEntry(p, size + DATA_OFFSET,
                          this->entryAddress[nextEntry - 1]);

    this->latestTerm = term;


    return this->nextEntry;
}

void Log::truncate(uint16_t index) {
    if (!validIndex(index) || (lastIndex() < index)) return;

    this->nextEntry  = index;
    this->latestTerm = this->getTerm(index);

    RASPFS.overwriteLog(this->data,
                        this->size(),
                        (uint8_t *)&this->entryAddress,
                        nextEntry * sizeof(this->entryAddress[0]));
}

uint16_t Log::size() {
    if (lastIndex() == 0) return 0;

    return this->entryAddress[nextEntry];
}

uint32_t Log::lastStoredTerm() {
    return this->latestTerm;
}

uint16_t Log::lastEntryAddress() {
    return !lastIndex() ? 0 : entryAddress[lastIndex() - 1];
}

logEntry_t * Log::getEntry(uint16_t index) {
    if (!validIndex(index) || (lastIndex() < index)) return NULL;

    logEntry_t *entry = new logEntry_t;

    uint8_t *p = getPointer(index);

    entry->term = getTermNumber(p);
    entry->size = (this->entryAddress[index] - this->entryAddress[index - 1]) -
                  DATA_OFFSET;
    entry->data = p + DATA_OFFSET;

    RASPDBG("returnig a logEntry_t at index: %d\n"
            "term: %d size: %d data: %s\n",
            index,
            entry->term,
            entry->size,
            entry->data);

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
    return !validIndex(index) ? NULL : &this->data[this->entryAddress[index - 1]];
}
