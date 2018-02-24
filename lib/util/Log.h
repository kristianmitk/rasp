#ifndef Log_h
#define Log_h

#include <Arduino.h>
#include "rasp_fs.h"


extern "C" {
    #include "stdint.h"
}

// using higher values causes memory problems and the boards begin to fail (i.e
// print stack trace and reset iself)
#define LOG_SIZE 33000

// we use a fixed array rather than dynamic structures to avoid memory
// fragmentation
#define NUM_LOG_ENTRIES 512

/**
 * Struct representing one dynamic sized log entry.
 * An entry stores:
 *      - the term in which the entry was created and appended to the log
 *      - the size of the data block
 *      - a pointer to the begining of the data block
 * This struct is only instanciated when a specific entry of the log is
 * requested by the `getEntry(uint16_t index)` function
 */
typedef struct LogEntry {
    uint32_t term;
    uint16_t size;
    void    *data;
} logEntry_t;


/**
 * The in memory representation of the Log. If you are looking for the
 * persistent one check the `rasp_fs.h` file there you find an interface to
 * write the data buffer of this log to a file.
 * This Log uses a fixed size byte buffer where entries may be stored.
 * In order to access an entry there is a adress (2-bytes per entry) array that
 * stores the proper offset where the corresponding entry is stored in the data
 * block.
 * [Log description]
 */
class Log {
public:

    /**
     * TODO: DOCS
     * [Log description]
     */
    Log();

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
     * [size description]
     * @return [description]
     */
    uint16_t    size();

    /**
     * TODO: DOCS
     * [lastTerm description]
     * @return [description]
     */
    uint32_t    lastStoredTerm();

    /**
     * TODO: DOCS
     * [printLastEntry description]
     * @return [description]
     */
    void        printLastEntry();

    /**
     * TODO: DOCS
     * [lastEntry description]
     * @return [description]
     */
    logEntry_t  lastEntry();

    /**
     * NOTE: we count log indexes begining at 1 and not with 0 like arrays
     * TODO: DOCS
     * [lastIndex description]
     * @return [description]
     */
    uint16_t    lastIndex();

    /**
     * TODO: DOCS
     * [getEntry description]
     * @param  index [description]
     * @return       [description]
     */
    logEntry_t* getEntry(uint16_t index);

    /**
     * TODO: DOCS
     * [getTerm description]
     * @param  index [description]
     * @return       [description]
     */
    uint32_t    getTerm(uint16_t index);


    /**
     * [lastEntryAddress description]
     * @return [description]
     */
    void truncate(uint16_t index);

private:

    /**
     * Retrieves the offset at which position in the byte buffer the start of
     * the last entry is stored.
     * @return [description]
     */
    uint16_t lastEntryAddress();

    /**
     * [getPointer description]
     * @param  index [description]
     * @return       [description]
     */
    uint8_t* getPointer(uint16_t index);

    uint16_t nextEntry;
    uint32_t latestTerm;

    // the actual log byte buffer
    uint8_t data[LOG_SIZE];

    // stores for every log entry the proper offset in the data[] buffer
    uint16_t entryAdress[NUM_LOG_ENTRIES];
};
#endif // ifndef Log_h
