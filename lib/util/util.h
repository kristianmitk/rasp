#ifndef util_h
#define util_h

#include <Arduino.h>

#define LOG_DATA_SIZE 128

typedef struct LogData {
    void    *head;
    uint16_t size;
} logData_t;


typedef struct LogEntry {
    uint32_t  term;
    logData_t data;
} logEntry_t;

#endif // ifndef util_h
