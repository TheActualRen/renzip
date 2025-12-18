#ifndef BLOCKTYPE1_H
#define BLOCKTYPE1_H

#include "bitwriter.h"
#include "lz77.h"

#include <stdio.h>

#define IS_LAST_BIT 0x01
#define IS_LAST_NUM_BITS 1

#define BTYPE1 0x01
#define BTYPE1_NUM_BITS 2

typedef enum
{
    B1_SUCCESS = 0,
    B1_HEADER_FAILURE,
    B1_INVALID_INPUT,
    B1_MALLOC_FAILURE,
    B1_READ_FAILURE,
    B1_WRITING_FAILURE,
    B1_FOOTER_FAILURE,
    B1_LZ77_FAILURE,
    B1_INVALID_BLOCKTYPE,
} B1_STATUS;


B1_STATUS write_fixed_huffman_block(BitWriter *bw, 
        const LZ77TokenList *tokens);

B1_STATUS blocktype1_encoding(FILE *input_file, FILE *output_file);

#endif
