#ifndef HUFFMAN_FIXED_H
#define HUFFMAN_FIXED_H

#include "bitreader.h"
#include "blocktype1.h"
#include "blocktype2.h"

#include <stdint.h>

#define NUM_LL_CODES 288
#define NUM_DIST_CODES 32

typedef struct
{
    uint16_t code;
    uint8_t bitlen;

} HuffmanFixedCode;

void init_fixed_huffman_tables(void);

void length_literal_to_code(uint16_t length, uint16_t *code, 
        uint8_t *offset_bits, uint16_t *extra_value);

void dist_literal_to_code(uint16_t dist, uint16_t *code,
        uint8_t *offset_bits, uint16_t *extra_value);

HuffmanFixedCode get_fixed_literal_code(uint16_t symbol);
HuffmanFixedCode get_fixed_dist_code(uint16_t dist);

void build_canonical_codes(const uint8_t *lengths, uint16_t *codes,
        int nsym);

B1_STATUS decode_length_symbol_b1(uint32_t sym, BitReader *br, uint16_t *out_length);
B1_STATUS decode_fixed_distance_symbol_b1(BitReader *br, uint16_t *out_sym);
B1_STATUS decode_distance_symbol_b1(uint32_t sym, BitReader *br, uint16_t *out_distance);
B1_STATUS decode_fixed_literal_or_length_b1(BitReader *br, uint32_t *out_sym);

uint32_t decode_fixed_literal_or_length(BitReader *br);
uint16_t decode_length_symbol(uint16_t sym, BitReader *br);
uint32_t decode_fixed_distance_symbol(BitReader *br);
uint16_t decode_distance_symbol(uint32_t sym, BitReader *br);

// B2_STATUS decode_length_symbol_b2(uint32_t sym, BitReader *br, uint16_t *out_length);
// B2_STATUS decode_fixed_distance_symbol_b2(BitReader *br, uint16_t *out_sym);
// B2_STATUS decode_distance_symbol_b2(uint32_t sym, BitReader *br, uint32_t *out_distance);
// B2_STATUS decode_fixed_literal_or_length_b2(BitReader *br, uint32_t *out_sym);

#endif
