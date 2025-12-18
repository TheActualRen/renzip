#ifndef BITREADER_H
#define BITREADER_H

#include <stddef.h>
#include <stdint.h>

typedef enum
{
    BITREADER_SUCCESS = 0,
    BITREADER_FAILED_INIT,
    BITREADER_FAILED_READ_BIT,
    BITREADER_FAILED_READ_BYTE,
    BITREADER_OUT_OF_DATA

} BITREADER_STATUS;

typedef struct
{
    uint8_t *buf;
    size_t buf_size;
    size_t byte_pos;

    uint8_t bit_buf;
    uint8_t bit_count;

} BitReader;

BITREADER_STATUS bitreader_init(BitReader *br, uint8_t *input_buf,
        size_t buf_size);

BITREADER_STATUS bitreader_read_bits(BitReader *br, uint32_t *bits, 
        uint8_t num_bits);

BITREADER_STATUS bitreader_read_bytes(BitReader *br, uint8_t *output_buf, size_t len);

BITREADER_STATUS bitreader_peek_bits(BitReader *br, uint32_t *out, uint8_t num_bits);

BITREADER_STATUS bitreader_drop_bits(BitReader *br, uint8_t num_bits);

#endif
