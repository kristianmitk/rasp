#ifndef Log_h
#define Log_h

#include <Arduino.h>
#include "rasp_fs.h"
#include "config.h"

/**
 * NOTE: Log entries are indexed from outside of this file begining from 1 but
 * internally like arrays from 0.
 */

/**
 * In the following the structure of a log entry is visualized:
 *
 * +----------------------------------------+
 * |        |       |                       |
 * -- term -- size -- data ------------------
 * 0        4       6                      size
 * |        |       |                       |
 * +----------------------------------------+
 *
 * -> Log entries may have dynamic sizes.
 */

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

    static Log& getInstance() {
        static Log instance;

        return instance;
    }

    /**
     * Appends a log entry to the data buffer if `NUM_LOG_ENTRIES` is not
     * exceeded as well as the `LOG_SIZE`.
     * The new log entry is also automatically appended to the persistent
     * storage (see rasp_fs.h)
     * @param term          term when the log was appended by the leader
     * @param data          pointer to the data to be appended
     * @param size          size of data to be appended
     * @return {uint16_t}   index position of appended entry
     */
    uint16_t append(uint32_t term,
                    uint8_t *data,
                    uint16_t size);


    /**
     * Returns the the sum of all log entry sized
     * @return {uint32_t}   sum of log entries sizes
     */
    uint16_t size();


    /**
     * Returns the last stored term number which is implicitly also the highest
     * term number
     * @return {uint32_t}       latest stored term number
     */
    uint32_t lastStoredTerm();


    /**
     * Prints the last log entry stored to the serial output
     * @return {void}
     */
    void printLastEntry();


    /**
     * Returns a logEntry_t struct with values of the last log entry stored in
     * the log
     * @return {logEntry_t}     pointer to the log entry struct
     */
    logEntry_t lastEntry();


    /**
     * Returns the index of the last entry that was stored in the log
     * @return {uint16_t}       index of last entry
     */
    uint16_t lastIndex();


    /**
     * Creates a logEntry_t struct with values specified by the index parameter
     * @param  index                index out of which the log entry is created
     * @return {logEntry_t}         pointer to the log entry struct
     */
    logEntry_t* getEntry(uint16_t index);


    /**
     * Returns the term number when the specified index was appended (the term
     * number of the leader that appended the entry is meant) or 0 if the index
     * does not exist.
     * @param  index            index of entry
     * @return {uint32_t}       term of requested index
     */
    uint32_t getTerm(uint16_t index);


    /**
     * Truncates the byte buffer so that everything after the specified index
     * is ignored and considered not existent. That means appending new entries
     * after this function was called leads to appending entries after the
     * specified index.
     */
    void truncate(uint16_t index);


    /**
     * Indicates wether an entry at the specified index exist or not.
     * @param  index        specifies the entry for which to check if it exsit
     * @return {bool}
     */
    bool exist(uint16_t index);

    /**
     * Creates the log by reading the persistent storage and loading whatever
     * is the content of the corresponding file. After that the `entryAddress`
     * array as well as `latestTerm` and `nextEntry` are filled with proper
     * values.
     */
    void initialize();

private:

    // due to singleton pattern the constructor is private
    Log() {}

    Log(Log const&);
    void     operator=(Log const&);

    /**
     * Retrieves the offset at which position in the byte buffer the start of
     * the last entry is stored.
     */
    uint16_t lastEntryAddress();


    /**
     * Returns a pointer to the requested entry specified by the `index`
     * parameter or NULL if the entry does not exist.
     * @param  index            specified the pointer to the entry to return
     * @return {uint8_t*}       pointer to the requested entry
     */
    uint8_t* getPointer(uint16_t index);


    uint16_t nextEntry;


    uint32_t latestTerm;

    // the actual data byte buffer
    uint8_t data[LOG_SIZE];

    // stores for every log entry the proper offset in the data[] buffer
    uint16_t entryAdress[NUM_LOG_ENTRIES];
};
#endif // ifndef Log_h
