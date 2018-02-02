#ifndef marshall_h
#define marshall_h

extern "C" {
    #include "stdint.h"
}

/**
 * TODO: DOCS
 * [unpack_uint32_t description]
 * @param  buf    [description]
 * @param  offset [description]
 * @return        [description]
 */
uint32_t unpack_uint32_t(uint8_t *buf,
                         int      offset);

/**
 * TODO: DOCS
 * [unpack_uint32_t description]
 * @param  buf    [description]
 * @param  offset [description]
 * @return        [description]
 */
uint16_t unpack_uint16_t(uint8_t *buf,
                         int      offset);


/**
 * TODO: DOCS
 * [unpack_uint32_t description]
 * @param  buf    [description]
 * @param  offset [description]
 * @return        [description]
 */
uint8_t unpack_uint8_t(uint8_t *buf,
                       int      offset);

/**
 * TODO: DOCS
 * [pack_uint32_t description]
 * @param buf    [description]
 * @param offset [description]
 * @param data   [description]
 */
void pack_uint32_t(uint8_t *buf,
                   int      offset,
                   uint32_t data);

/**
 * TODO: DOCS
 * [pack_uint32_t description]
 * @param buf    [description]
 * @param offset [description]
 * @param data   [description]
 */
void pack_uint16_t(uint8_t *buf,
                   int      offset,
                   uint16_t data);


/**
 * TODO: DOCS
 * [pack_uint8_t description]
 * @param buf    [description]
 * @param offset [description]
 * @param data   [description]
 */
void pack_uint8_t(uint8_t *buf,
                  int      offset,
                  uint8_t  data);


#endif // ifndef marshall_h
