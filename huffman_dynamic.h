#ifndef HUFFMAN_DYNAMIC_H
#define HUFFMAN_DYNAMIC_H

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

#endif
