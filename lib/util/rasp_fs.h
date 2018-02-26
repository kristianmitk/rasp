#ifndef rasp_fs_h
#define rasp_fs_h

#include <Arduino.h>
#include <fs.h>
#include "marshall.h"
#include "env.h"


#define CURRENT_TERM    "/SS/currentTerm"
#define VOTED_FOR       "/SS/votedFor"
#define LOG             "/SS/log"

/**
 * A wrapper that handles communication with the SPIFFS file system.
 * Provides easy to use functions to write and read data that needs to be
 * stored persistently.
 * This class follows the static singleton pattern and thus is instanciated only
 * once.
 * To get the static object call: `RASPFS::getInstance()`
 *
 */

class RASPFS {
public:

    /**
     * To access the singleton object. The class is lazily instantiated when
     * this function is called the first time
     *
     * @return     {RASPFS}     reference to the static instance object
     */
    static RASPFS& getInstance()
    {
        static RASPFS instance;

        return instance;
    }

    /**
     * Getter function to return the content of the `CURRENT_TERM` file.
     * @return {uint32_t}   number that is stored in the `CURRENT_TERM` file
     */
    uint32_t readCurrentTerm() {
        return read_uint32_t(CURRENT_TERM);
    }

    /**
     * Getter function to return the content of the `VOTED_FOR` file.
     * @return {uint32_t}   number that is stored in the `VOTED_FOR` file.
     */
    uint32_t readVotedFor() {
        return read_uint32_t(VOTED_FOR);
    }

    /**
     * Setter function to overwrite the content of the `VOTED_FOR` file.
     * @return {uint32_t}   number that is was written to the `VOTED_FOR` file.
     */
    uint32_t writeVotedFor(uint32_t id) {
        return write_uint32_t(VOTED_FOR, id);
    }

    /**
     * Setter function to overwrite the content of the `CURRENT_TERM` file.
     * @return {uint32_t}   number that is was written to the `CURRENT_TERM`
     *                      file.
     */
    uint32_t writeCurrentTerm(uint32_t term) {
        return write_uint32_t(CURRENT_TERM, term);
    }

    /**
     * Appends a new log entry to the `LOG` file.
     * @param  data     pointer to the data block that has to be appended
     * @param  size     size of bytes to write
     * @return          size of bytes that were written
     *
     * TODO: remove/preprocess console prints
     */
    size_t appendLogEntry(uint8_t *data, uint16_t size) {
        if (!SPIFFS.exists(LOG)) {
            Serial.printf("%s does not exist", LOG);
            openLog();
        }
        size_t written = 0;

        Serial.printf("Before: %lu\n", millis());

        written = _logF.write(data, size);

        Serial.printf("Written: %lu bytes to LOG. New file size: %lu bytes\n",
                      written,
                      _logF.size());
        Serial.printf("After: %lu\n", millis());

        return written;
    }

    /**
     * Reads whatever is stored in the `LOG` file and puts the content into the
     * by the function parameter pointer specified data block.
     * This function is called only once and that is in the constructor of the
     * `Log` class.
     *
     * @param  buf          pointer to data block where to load the content
     * @return {size_t}     number of bytes read from `LOG` file
     */
    size_t readLog(uint8_t *buf) {
        Serial.printf("File log size: %lu\n", _logF.size());
        return _logF.read(buf, _logF.size());
    }

    /**
     * Since SPIFFS does not offer a truncate operation on files, this function
     * is needed for the one and only case; if there are uncomitted log entries
     * persistently stored that should be replaced by new ones at the same
     * index.
     * @param  buf          pointer to data block that should overwrite the
     *                      content of the `LOG` file
     * @param  size         size of data block
     * @return {size_t}     number of bytes written to `LOG` file
     */
    size_t overwriteLog(uint8_t *buf, uint16_t size) {
        Serial.printf("Log size before overwriting: %lu\n", _logF.size());

        File   tmp     = SPIFFS.open(LOG, "w");
        size_t written = 0;

        written = tmp.write(buf, size);
        Serial.printf("Log size after overwriting: %lu\n", tmp.size());
        tmp.close();

        return written;
    }

private:

    /**
     * Overwrites the content of the `filename` file with the specified unsigned
     * integer `val`.
     * @param filename          path of file to which to write
     * @param val {uint32_t}    value that has been written
     */
    uint32_t write_uint32_t(const char *filename, uint32_t val) {
        uint8_t buf[4];
        File    f = SPIFFS.open(filename, "w");

        pack_uint32_t(buf, 0, val);
        f.write(buf, 4);
        f.close();
        return val;
    }

    /**
     * Interprets the first four bytes as an unsigned integer of the by the
     * `filename` specified file and returns the resulting value.
     * @param  filename         path of file from which to read
     * @return {uint32_t}       value that is stored under the corresponding
     *                          file
     */
    uint32_t read_uint32_t(const char *filename) {
        uint8_t buf[4];
        File    f = SPIFFS.open(filename, "r");

        f.read(buf, 4);
        f.close();
        return unpack_uint32_t(buf, 0);
    }

    /**
     * Opens the `LOG` file in 'a+' mode.
     */
    void openLog() {
        _logF = SPIFFS.open(LOG, "a+");
    }

    /**
     * Prints some informations about SPIFFS.
     * Those are:
     *  - max open files
     *  - block size
     *  - max file size length
     *  - page size
     *  - used bytes
     *  - total bytes
     */
    void printSPIFFSInfo() {
        FSInfo info;

        SPIFFS.info(info);

        Serial.printf(
            "[INFO] about SPIFFS:\n"
            "Max open files: %lu\n"
            "Block size: %lu\n"
            "Path length: %lu\n"
            "Page size: %lu\n"
            "Used bytes: %lu\n"
            "Max bytes: %lu\n\n",
            info.maxOpenFiles,
            info.blockSize,
            info.maxPathLength,
            info.pageSize,
            info.usedBytes,
            info.totalBytes);
    }

    /**
     * Initialized the SPIFFS file system and in case the `INITIAL_SETUP`
     * preprocessor variable is set, it removes all existent files and creates
     * only those that are specified in the RASP_FILES enum.
     */
    RASPFS() {
        if (!SPIFFS.begin()) {
            Serial.println("\n[ERR] Could not mount SPIFF file system");
            exit;
        }
        Serial.println("\n[SUCC] File System mounted");

        printSPIFFSInfo();

#ifdef INITIAL_SETUP

        Serial.println("[INFO] Running initial FS setup");
        Dir dir = SPIFFS.openDir("/");

        while (dir.next()) {
            Serial.print("About to remove \"" + dir.fileName() + "\"");
            Serial.printf(" with size: %lu\n", dir.fileSize());
            SPIFFS.remove(dir.fileName());
        }

        File f;

        for (uint8_t i = 0; i < 3; i++) {
            f = SPIFFS.open(FILE_NAME[i], "w");
            Serial.printf("Created empty file \"%s\" \n", f.name());
            f.close();
        }

        write_uint32_t(FILE_NAME[CURRENT_TERM], 0);
        write_uint32_t(FILE_NAME[VOTED_FOR],    0);

        Serial.println();

#endif // ifdef INITIAL_SETUP

        // logPart = 1;
        this->openLog();
    }

    // restrict usage of this class.
    RASPFS(RASPFS const&);


    void operator=(RASPFS const&);


    File _logF;

    // We separate the log into three files. Given the fact that SPIFFS gets
    // slow when a file reaches 20kb we avoid this problem by actually using
    // several files for the log
    // TODO: implement this functionality
    // uint8_t logPart;
};

#endif // ifndef rasp_fs_h
