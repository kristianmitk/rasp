#ifndef util_h
#define util_h

#include <Arduino.h>

#define LOG_DATA_SIZE 128

typedef struct LogEntry {
    uint32_t term;
    uint8_t  data[LOG_DATA_SIZE];
} logEntry_t;

#endif // ifndef util_h
