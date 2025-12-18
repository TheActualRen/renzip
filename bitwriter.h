#ifndef BITWRITER_H
#define BITWRITER_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

typedef enum {
    BITWRITER_SUCCESS = 0,
    BITWRITER_INIT_FAILURE,
    BITWRITER_PUSH_BIT_FAILURE,
    BITWRITER_PUSH_BYTE_FAILURE,
    BITWRITER_WRITE_FAILURE,
    BITWRITER_FLUSH_FAILURE

} BITWRITER_STATUS;

typedef struct
{
    uint8_t *buf;
    size_t buf_size;
    size_t byte_pos;
    
    uint8_t bit_buf;
    uint8_t bit_count;

} BitWriter;

BITWRITER_STATUS bitwriter_init(BitWriter *bw, uint8_t *output_buf, size_t buf_size);
BITWRITER_STATUS bitwriter_push_bits(BitWriter *bw, uint32_t bits, uint8_t num_bits);
BITWRITER_STATUS bitwriter_push_bytes(BitWriter *bw, const uint8_t *data, size_t len);
BITWRITER_STATUS bitwriter_write_bits(BitWriter *bw, FILE *output_file);
BITWRITER_STATUS bitwriter_flush(BitWriter *bw, size_t *bytes_written);

#endif
