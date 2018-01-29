#ifndef Log_h
#define Log_h

#include <Arduino.h>
#include "rasp_fs2.h"
#include "util.h"

extern "C" {
    #include "stdint.h"
}

#define LOG_DATA_SIZE 128
#define LOG_LENGTH 256

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
    void       append(logEntry_t newEntry);

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
    size_t     size();

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

    logEntry_t lastEntry();

private:

    uint8_t nextEntry;
    uint32_t latestTerm;

    // TODO: solve fixed array size - maybe use std::vector ?!
    logEntry_t entries[LOG_LENGTH];
};
#endif // ifndef Log_h
