#ifndef rasp_fs_h
#define rasp_fs_h
#include <Arduino.h>
#include <fs.h>
#include "marshall.h"
#include "util.h"
#include "env.h"

/**
 * TODO: DOCS
 * [getInstance description]
 * @return [description]
 */
class RASPFS {
public:

    /**
     * TODO: DOCS
     * [getInstance description]
     * @return [description]
     */
    enum RASP_File: uint8_t {
        CURRENT_TERM = 0,
        VOTED_FOR    = 1,
        LOG          = 2
    };


    /**
     * To access the singleton object. The class is instantiated when this
     * function is called the first time
     * TODO: DOCS
     * [getInstance description]
     * @return [description]
     */
    static RASPFS& getInstance()
    {
        static RASPFS instance;

        return instance;
    }

    /**
     * TODO: DOCS
     * [readCurrentTerm description]
     * @return [description]
     */
    uint32_t readCurrentTerm() {
        return read_uint32_t(FILE_NAME[CURRENT_TERM]);
    }

    /**
     * TODO: DOCS
     * [readCurrentTerm description]
     * @return [description]
     */
    uint32_t readVotedFor() {
        return read_uint32_t(FILE_NAME[VOTED_FOR]);
    }

    /**
     * TODO: DOCS
     * [write description]
     * @param  f    [description]
     * @param  data [description]
     * @return      [description]
     */
    uint32_t write(RASP_File f, uint32_t data) {
        return write_uint32_t(FILE_NAME[f], data);
    }

    /**
     * TODO: DOCS
     * [read description]
     * @param  f [description]
     * @return   [description]
     */
    uint32_t read(RASP_File f) {
        return read_uint32_t(FILE_NAME[f]);
    }

    /**
     * TODO: DOCS
     * [appendLogEntry description]
     * @param  newEntry [description]
     * @return          [description]
     */
    size_t appendLogEntry(uint8_t *data, uint16_t size) {
        if (!SPIFFS.exists(FILE_NAME[LOG])) {
            Serial.printf("%s does not exist", FILE_NAME[LOG]);
            openLog();
        }

        // TODO: add error handling
        Serial.printf("Before: %lu\n", millis());
        _logF.write(data, size);

        // Serial.printf("Written: %lu bytes to LOG. New file size: %lu
        // bytes\n",
        //               _logF.write(data, size),
        //               _logF.size());
        Serial.printf("After: %lu\n", millis());
        return size;
    }

    /**
     * TODO: DOCS
     * [remove description]
     * @param  f [description]
     * @return   [description]
     */

    // size_t remove(RASP_File f) {
    //     return SPIFFS.remove(FILE_NAME[f]);
    // }

private:

    /**
     * TODO: DOCS
     * [write_uint32_t description]
     * @param filename [description]
     * @param data     [description]
     */
    uint32_t write_uint32_t(const char *filename, uint32_t data) {
        uint8_t buf[4];
        File    f = SPIFFS.open(filename, "w");

        pack_uint32_t(buf, 0, data);
        f.write(buf, 4);
        f.close();
        return data;
    }

    /**
     * TODO: DOCS
     * [read_uint32_t description]
     * @param  filename [description]
     * @return          [description]
     */
    uint32_t read_uint32_t(const char *filename) {
        uint8_t buf[4];
        File    f = SPIFFS.open(filename, "r");

        f.read(buf, 4);
        f.close();
        return unpack_uint32_t(buf, 0);
    }

    /**
     * TODO: DOCS
     * [Log::openLog description]
     */
    void openLog() {
        // TODO check on sizes and open `SS/LOG+(1|2|3)`
        _logF = SPIFFS.open(FILE_NAME[LOG], "a+");
    }

    /**
     * TODO: DOCS
     * [printSPIFFSInfo description]
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
     * TODO: DOCS
     * [RASPFS description]
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
        Dir  dir = SPIFFS.openDir("/");
        File f;

        while (dir.next()) {
            f = SPIFFS.open(dir.fileName(), "w");
            Serial.printf("%s %lu bytes\n", f.name(), f.size());
            f.close();
        }

        write_uint32_t(FILE_NAME[CURRENT_TERM], 0);
        write_uint32_t(FILE_NAME[VOTED_FOR],    0);
        Serial.println();
#endif // ifdef INITIAL_SETUP

        this->openLog();
    }

    RASPFS(RASPFS const&);
    void operator=(RASPFS const&);

    const char *FILE_NAME[3] = {
        "/SS/currentTerm",
        "/SS/votedFor",
        "/SS/log"
    };

    File _logF;
};

#endif // ifndef rasp_fs_h
