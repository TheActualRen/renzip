#ifndef HUFFMAN_FIXED_H
#define HUFFMAN_FIXED_H

#include <stdint.h>

typedef struct {
	uint16_t code;
	uint8_t bitlen;
} HuffmanFixedCode;

void init_fixed_huffman_tables(void);
HuffmanFixedCode get_fixed_literal_code(uint16_t symbol);
HuffmanFixedCode get_fixed_dist_code(uint16_t dist);

#endif
