#ifndef blocktype2_h
#define blocktype2_h

#include "bitwriter.h"
#include "lz77.h"
#include "rle.h"

#include <stdint.h>
#include <stdio.h>

#define IS_LAST_BIT 0x01
#define IS_LAST_NUM_BITS 1

#define BTYPE2 0x02
#define BTYPE2_NUM_BITS 2

typedef enum
{
    B2_SUCCESS = 0,
    B2_HEADER_FAILURE,
    B2_INVALID_INPUT,
    B2_MALLOC_FAILURE,
    B2_READ_FAILURE,
    B2_WRITING_FAILURE,
    B2_FOOTER_FAILURE,
    B2_LZ77_FAILURE,
    B2_INVALID_BLOCKTYPE,
    B2_DECODE_FAILURE

} B2_STATUS;

typedef struct
{
    const uint16_t *ll_codes;
    const uint8_t *ll_lengths;
    const uint16_t *dist_codes;
    const uint8_t *dist_lengths;

} HuffmanEmitTables;

typedef struct
{
    uint16_t ll_codes[286];
    uint8_t ll_lengths[286];
    uint16_t hlit;

    uint16_t dist_codes[30];
    uint8_t dist_lengths[30];
    uint16_t hdist;

    uint16_t cl_codes[19];
    uint8_t cl_lengths[19];
    uint16_t hclen;

    RLEItem *rle_seq;
    size_t rle_len;

} DynamicHuffmanTables;

B2_STATUS build_dynamic_huffman_tables(const LZ77TokenList *tokens,
                                       DynamicHuffmanTables *t);

B2_STATUS emit_dynamic_huffman_tokens(BitWriter *bw,
                                      const LZ77TokenList *tokens,
                                      const HuffmanEmitTables *tables);

B2_STATUS emit_dynamic_huffman_header(BitWriter *bw, DynamicHuffmanTables *t);

B2_STATUS blocktype2_encoding(FILE *input_file, FILE *output_file);
B2_STATUS blocktype2_decoding(FILE *input_file, FILE *output_file);

#endif
