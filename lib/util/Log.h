#ifndef Log_h
#define Log_h

#include <Arduino.h>
#include "rasp_fs2.h"
#include "util.h"

extern "C" {
    #include "stdint.h"
}

#define LOG_SIZE 33000
#define NUM_LOG_ENTRIES 512

class Log {
public:

    /**
     * TODO: DOCS
     * [Log description]
     */
    Log() {
        // TODO: constructor should read from disk and set proper values
        nextEntry  = 0;
        latestTerm = 0;
    }

    /**
     * TODO: DOCS
     * [append description]
     * @param  newEntry [description]
     * @return          [description]
     */
    void append(uint32_t term,
                uint8_t *data,
                uint16_t size);

    /**
     * TODO: DOCS
     * [readEntry description]
     * @param  index [description]
     * @return       [description]
     */
    logEntry_t read(uint32_t index);

    /**
     * TODO: DOCS
     * [applyToStateMachine description]
     * @param  index [description]
     * @return       [description]
     */
    void       applyToStateMachine(uint32_t index);

    /**
     * TODO: DOCS
     * [size description]
     * @return [description]
     */
    uint16_t   size();

    /**
     * TODO: DOCS
     * [lastTerm description]
     * @return [description]
     */
    uint32_t   lastStoredTerm();

    /**
     * TODO: DOCS
     * [printLastEntry description]
     * @return [description]
     */
    void       printLastEntry();

    /**
     * TODO: DOCS
     * [lastEntry description]
     * @return [description]
     */
    logEntry_t lastEntry();

    /**
     * TODO: DOCS
     * [lastIndex description]
     * @return [description]
     */
    uint16_t   lastIndex();

    logEntry_t getEntry(uint16_t index);

private:

    uint16_t lastEntryAddress();
    uint8_t* getPointer(uint16_t index);
    uint16_t nextEntry;
    uint32_t latestTerm;

    uint8_t data[LOG_SIZE];
    uint16_t dataPointers[NUM_LOG_ENTRIES];
};
#endif // ifndef Log_h
