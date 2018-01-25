#ifndef rasp_fs_h
#define rasp_fs_h
#include <Arduino.h>
#include <fs.h>
#include "marshall.h"

#define CURRENT_TERM_FILE_NAME  "/SS/currentTerm"
#define VOTED_FOR_FILE_NAME     "/SS/votedFor"
#define LOG_FILE_NAME           "/SS/log"

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
     * [writeCurrentTerm description]
     * @param  term [description]
     * @return      [description]
     */
    uint32_t writeCurrentTerm(uint32_t term) {
        uint8_t buf[4];

        pack_uint32_t(buf, 0, term);
        this->currentTerm.write(buf, 4);
        this->currentTerm.flush();

        // Serial.printf("File size: %lu, Position: %d\n",
        //               this->currentTerm.size(),
        //               this->currentTerm.position());

        // point to the begining of the file
        this->currentTerm.seek(0, SeekSet);

        Serial.printf("Called term: %lu,\nSaved Term: %lu\n",
                      term,
                      readCurrentTerm());
        return term;
    }

    /**
     * TODO: DOCS
     * [readCurrentTerm description]
     * @return [description]
     */
    uint32_t readCurrentTerm() {
        char buf[4];

        Serial.printf("File size: %lu, Position: %d\n",
                      this->currentTerm.size(),
                      this->currentTerm.position());

        this->currentTerm.readBytes(buf, 4);
        this->currentTerm.seek(0, SeekSet);
        return unpack_uint32_t((uint8_t *)buf, 0);
    }

private:

    RASPFS() {
        if (!SPIFFS.begin()) {
            Serial.println("\nCould not mount SPIFF file system");
            exit;
        } else {
            Serial.println("\nFile System mounted");
        }
        votedFor    = SPIFFS.open(VOTED_FOR_FILE_NAME, "w+");
        currentTerm = SPIFFS.open(CURRENT_TERM_FILE_NAME, "w+");
        log         = SPIFFS.open(LOG_FILE_NAME, "w+");
    }

    RASPFS(RASPFS const&);
    void operator=(RASPFS const&);

    File votedFor;
    File currentTerm;
    File log;
};

#endif // ifndef rasp_fs_h
