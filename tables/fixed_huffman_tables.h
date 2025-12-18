#ifndef FIXED_HUFFMAN_TABLES_H
#define FIXED_HUFFMAN_TABLES_H

#include <stdint.h>

#define NUM_LL_CODES   288
#define NUM_DIST_CODES 32

extern const uint16_t ll_range_start[29];
extern const uint8_t  ll_offset_bits[29];

extern const uint16_t dist_range_start[30];
extern const uint8_t  dist_offset_bits[30];

#endif
