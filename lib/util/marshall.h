#ifndef marshall_h
#define marshall_h

extern "C" {
    #include "stdint.h"
}

/*******************************************************************************
 * NOTE: We follow the chip endianess i.e the little-endian and do not marshall
 * into network byte order i.e big-endian
 ******************************************************************************/


/**
 * Unpacks a unsigned 32-bit integer that is stored in a byte buffer and returns
 * the value
 * @param  buf              pointer to data
 * @param  offset           offset (in bytes) where to read from
 * @return {uint32_t}
 */
uint32_t unpack_uint32_t(uint8_t *buf,
                         int      offset);

/**
 * Unpacks a unsigned 16-bit integer that is stored in a byte buffer and returns
 * the value
 * @param  buf              pointer to data
 * @param  offset           offset (in bytes) where to read from
 * @return {uint16_t}
 */
uint16_t unpack_uint16_t(uint8_t *buf,
                         int      offset);


/**
 * Unpacks a unsigned 8-bit integer that is stored in a byte buffer and returns
 * the value
 * @param  buf              pointer to data
 * @param  offset           offset (in bytes) where to read from
 * @return {uint8_t}
 */
uint8_t unpack_uint8_t(uint8_t *buf,
                       int      offset);

/**
 * Packs a unsigned 32-bit integer into the `buf` byte buffer at the specified
 * `offset` position
 * @param  buf              pointer to data
 * @param  offset           offset (in bytes) where to read from
 * @param  data             value to be stored
 */
void pack_uint32_t(uint8_t *buf,
                   int      offset,
                   uint32_t data);

/**
 * Packs a unsigned 16-bit integer into the `buf` byte buffer at the specified
 * `offset` position
 * @param  buf              pointer to data
 * @param  offset           offset (in bytes) where to read from
 * @param  data             value to be stored
 */
void pack_uint16_t(uint8_t *buf,
                   int      offset,
                   uint16_t data);


/**
 * Packs a unsigned 8-bit integer into the `buf` byte buffer at the specified
 * `offset` position
 * @param  buf              pointer to data
 * @param  offset           offset (in bytes) where to read from
 * @param  data             value to be stored
 */
void pack_uint8_t(uint8_t *buf,
                  int      offset,
                  uint8_t  data);


#endif // ifndef marshall_h
