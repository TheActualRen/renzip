#ifndef BITWRITER_H
#define BITWRITER_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define BITWRITER_SUCCESS 0
#define BITWRITER_FAILED_INIT 1
#define BITWRITER_FAILED_PUSH_BIT 2
#define BITWRITER_FAILED_PUSH_BYTE 3
#define BITWRITER_FAILED_WRITE 4
#define BITWRITER_FAILED_FLUSH 5

typedef struct
{
    uint8_t *buf;
    size_t buf_size;
    size_t byte_pos;

    uint8_t bit_buf;
    uint8_t bit_count;

} BitWriter;

int bitwriter_init(BitWriter *bw, uint8_t *output_buf, size_t buf_size);

int bitwriter_push_bits(BitWriter *bw, uint32_t bits, uint8_t num_bits);

int bitwriter_push_bytes(BitWriter *bw, const uint8_t *data, size_t len);

int bitwriter_write_bits(BitWriter *bw, FILE *output_file);

int bitwriter_flush(BitWriter *bw, size_t *bytes_written);

#endif
