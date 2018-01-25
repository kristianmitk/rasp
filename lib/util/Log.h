#ifndef Log_h
#define Log_h

#include <Arduino.h>
#include <fs.h>
#include "util.h"

extern "C" {
    #include "stdint.h"
}

#define LOG_SIZE 1024

typedef struct LogEntry {
    uint32_t term;
    uint8_t *command;
} LogEntry;

class Log {
public:

    /**
     * TODO: DOCS
     * [Log description]
     */
    Log() {
        // TODO: constructor should read from disk first and set proper values
        latestIndex = 0;
        latestTerm  = 0;
    }

    uint32_t latestIndex;
    uint32_t latestTerm;

    // TODO: solve fixed array size - maybe use std::vector ?!
    LogEntry entries[LOG_SIZE];

    /**
     * TODO: DOCS
     * [appendEntry description]
     */
    void     appendEntry();

    /**
     * TODO: DOCS
     * [readEntry description]
     * @param  index [description]
     * @return       [description]
     */
    LogEntry readEntry(uint32_t index);

    /**
     * TODO: DOCS
     * [applyToStateMachine description]
     * @param  index [description]
     * @return       [description]
     */
    void     applyToStateMachine(uint32_t index);
};
#endif // ifndef Log_h
