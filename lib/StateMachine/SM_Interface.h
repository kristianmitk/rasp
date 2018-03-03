#ifndef SM_Inderface_h
#define SM_Inderface_h

#include <Arduino.h>

class SM_Interface {
public:

    virtual void     apply(const void *data) = 0;
    virtual uint8_t* read(const void *data)  = 0;
};

#endif // ifndef SMInderface_h
