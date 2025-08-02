#include "../include/huffman_fixed.h"

static HuffmanFixedCode literal_table[286];
static HuffmanFixedCode dist_table[32];

static uint16_t reverse_bits(uint16_t code, uint8_t bitlen) {
	uint16_t reversed = 0;

	for (uint8_t i = 0; i < bitlen; i++) {
		reversed = (reversed << 1) | (code & 1);
		code >>= 1;
	}

	return reversed;
}

void init_fixed_huffman_tables(void) {
	for (uint16_t i = 0; i <= 143; i++) {
		literal_table[i].bitlen = 8;
		literal_table[i].code = reverse_bits(0x30 + i, 8);
	}

	for (uint16_t i = 144; i <= 255; i++) {
		literal_table[i].bitlen = 9;
		literal_table[i].code = reverse_bits(0x190 + (i - 144), 9);
	}

	for (uint16_t i = 256; i <= 279; i++) {
		literal_table[i].bitlen = 7;
		literal_table[i].code = reverse_bits(i - 256, 7);
	}

	for (uint16_t i = 280; i <= 285; i++) {
		literal_table[i].bitlen = 8;
		literal_table[i].code = reverse_bits(0xC0 + (i - 280), 8);
	}

	for (uint16_t i = 0; i < 32; i++) {
		dist_table[i].bitlen = 5;
		dist_table[i].code = reverse_bits(i, 5);
	}
}

HuffmanFixedCode get_fixed_literal_code(uint16_t symbol) {
	return literal_table[symbol];
}

HuffmanFixedCode get_fixed_dist_code(uint16_t dist) {
	return dist_table[dist];
}
