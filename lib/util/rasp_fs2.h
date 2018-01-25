#ifndef rasp_fs_h
#define rasp_fs_h
#include <Arduino.h>
#include <fs.h>
#include "marshall.h"


enum RASP_File {
    CURRENT_TERM = 0,
    VOTED_FOR    = 1,
    LOG          = 2
};

/**
 * TODO: DOCS
 * [getInstance description]
 * @return [description]
 */
class RASPFS {
public:

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

    uint32_t read_uint32_t(const char *filename) {
        char buf[4];
        File f = SPIFFS.open(filename, "r");

        f.readBytes(buf, 4);
        f.close();
        return unpack_uint32_t((uint8_t *)buf, 0);
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
    }

    RASPFS(RASPFS const&);
    void operator=(RASPFS const&);

    const char *FILE_NAME[3] = {
        "/SS/currentTerm",
        "/SS/votedFor",
        "/SS/log"
    };
};

#endif // ifndef rasp_fs_h
