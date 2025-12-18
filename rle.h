#ifndef RLE_H
#define RLE_H

#include <stdint.h>
#include <stddef.h>

#define MAX_OUTPUT_CAP 600

typedef struct
{
    uint8_t symbol;
    uint8_t offset_bits;
    uint16_t extra_value;

} RLEItem;

void rle_encode(const uint8_t *input_buf, size_t len,
        RLEItem *output_buf, size_t *output_len);

#endif
