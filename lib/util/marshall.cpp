#include "marshall.h"

uint32_t unpack_uint32_t(uint8_t *buf, int offset) {
    return (uint32_t)buf[offset + 3] << 24 |
           (uint32_t)buf[offset + 2] << 16 |
           (uint32_t)buf[offset + 1] << 8  |
           (uint32_t)buf[offset];
}

uint32_t unpack_uint8_t(uint8_t *buf, int offset) {
    return (uint32_t)buf[offset];
}

void pack_uint32_t(uint8_t *buf, int offset, uint32_t data) {
    int8_t *p = (int8_t *)&data;

    for (int i = 0; i < 4; i++) {
        buf[offset + i] = p[0 + i];
    }
}

void pack_uint8_t(uint8_t *buf, int offset, uint8_t data) {
    buf[offset] = data;
}
