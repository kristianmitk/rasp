#ifndef SM_Inderface_h
#define SM_Inderface_h

#include <Arduino.h>

typedef struct smData {
    uint8_t *data;
    size_t   size;
} smData_t;

// TODO: DOCS

class SM_Interface {
public:

    virtual smData_t* apply(const void *data,
                            size_t      dataSize) = 0;
    virtual smData_t* read(const void *data,
                           size_t      dataSize) = 0;

    smData_t smRes;
};

#endif // ifndef SMInderface_h
