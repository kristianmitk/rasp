#ifndef rasp_fs_h
#define rasp_fs_h
#include <Arduino.h>
#include <fs.h>
#include "marshall.h"
#include "util.h"

#define LOGENTRY_SIZE 132

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
    uint32_t write(uint8_t f, uint32_t data) {
        return write_uint32_t(FILE_NAME[f], data);
    }

    /**
     * TODO: DOCS
     * [read description]
     * @param  f [description]
     * @return   [description]
     */
    uint32_t read(uint8_t f) {
        return read_uint32_t(FILE_NAME[f]);
    }

    /**
     * TODO: DOCS
     * [appendString description]
     * @param  name   [description]
     * @param  string [description]
     * @return        [description]
     */
    size_t appendString(RASP_File name = LOG, const char *string = NULL) {
        File f = SPIFFS.open(FILE_NAME[name], "a");

        f.write((uint8_t *)string, strlen(string));
        f.close();
    }

    /**
     * TODO: DOCS
     * [appendLogEntry description]
     * @param  newEntry [description]
     * @return          [description]
     */
    size_t appendLogEntry(logEntry_t newEntry) {
        serializeLogEntry(newEntry);
        File f = SPIFFS.open(FILE_NAME[LOG], "a");

        f.write(this->serializeBuffer, LOGENTRY_SIZE);
        Serial.printf("New file size is: %lu\n",
                      f.size());
        f.close();
        return LOGENTRY_SIZE;
    }

    size_t remove(RASP_File f) {
        return SPIFFS.remove(FILE_NAME[f]);
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

    /**
     * TODO: DOCS
     * [read_uint32_t description]
     * @param  filename [description]
     * @return          [description]
     */
    uint32_t read_uint32_t(const char *filename) {
        char buf[4];
        File f = SPIFFS.open(filename, "r");

        f.readBytes(buf, 4);
        f.close();
        return unpack_uint32_t((uint8_t *)buf, 0);
    }

    /**
     * TODO: DOCS
     * [serialize description]
     * @param  t [description]
     * @return   [description]
     */
    template<typename T>
    uint8_t* serialize(const T& t) {
        uint8_t *buffer[sizeof(t)];

        memcpy(buffer, t, sizeof(t));
        return buffer;
    }

    /**
     * TODO: DOCS
     * [serializeLogEntry description]
     * @param  logEntry [description]
     * @param  buffer   [description]
     * @return          [description]
     */
    uint8_t* serializeLogEntry(logEntry_t logEntry) {
        clearBuffer();
        uint32_t *p32 = (uint32_t *)serializeBuffer;

        *p32 = logEntry.term;
        p32++;

        uint8_t *p8 = (uint8_t *)p32;
        int i       = 0;

        while (i < LOG_DATA_SIZE) {
            *p8 = logEntry.data[i];
            p8++;
            i++;
        }
    }

    // /**
    //  * TODO: DOCS
    //  * [deserialize description]
    //  * @param  buf [description]
    //  * @return     [description]
    //  */
    // template<typename T>
    // const T& deserialize(const uint8_t buf) {
    //     T t;
    //
    //     memcpy(t, buf, sizeof(t));
    //     return t;
    // }

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

    void clearBuffer() {
        memset(serializeBuffer, 0, LOG_DATA_SIZE);
    }

    uint8_t serializeBuffer[132];
};

#endif // ifndef rasp_fs_h
