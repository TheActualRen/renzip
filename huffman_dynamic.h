#ifndef HUFFMAN_DYNAMIC_H
#define HUFFMAN_DYNAMIC_H

#include "bitreader.h"
#include "blocktype2.h"

#include <stdint.h>

#define MAX_CODELEN 15 

typedef struct HuffmanDynamicCode
{
    uint16_t symbol;
    uint32_t freq;

    struct HuffmanDynamicCode *left;
    struct HuffmanDynamicCode *right;

} HuffmanDynamicCode;

HuffmanDynamicCode *build_huffman_tree(uint32_t *freqs, int nsym);

void generate_code_lengths(HuffmanDynamicCode *node, 
        uint8_t *code_lengths, int depth);

void free_huffman_tree(HuffmanDynamicCode *node);

uint32_t decode_dynamic_distance_symbol(BitReader *br, HuffmanDynamicCode *dist_tree);

HuffmanDynamicCode *build_huffman_tree_from_lengths(uint8_t *lengths, int nsym);

uint32_t decode_dynamic_literal_or_length(BitReader *br, HuffmanDynamicCode *tree);
 
B2_STATUS decode_dynamic_literal_or_length_b2(BitReader *br, HuffmanDynamicCode *ll_tree, uint32_t *out_sym);
B2_STATUS decode_dynamic_distance_symbol_b2(BitReader *br, HuffmanDynamicCode *dist_tree, uint16_t *out_sym);

#endif
