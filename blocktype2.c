#include "blocktype2.h"
#include "header.h"
#include "lz77.h"
#include "bitwriter.h"
#include "huffman_fixed.h"
#include "huffman_dynamic.h"
#include "rle.h"
#include "footer.h"

#include <stdlib.h>
#include <string.h>

static const uint8_t CL_ORDER[] = {
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

int blocktype2_encoding(FILE *input_file, FILE *output_file)
{

    if (write_gzip_header(output_file) != GZIP_HEADER_SUCCESS)
    {
        fprintf(stderr, "Error writing gzip header, Block Type 0\n");
        return BLOCKTYPE2_HEADER_FAILURE;
    }

    fseek(input_file, 0, SEEK_END);
    size_t input_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);


    uint8_t *input_buf = malloc(input_size);

    if (!input_buf)
    {
        fprintf(stderr, "Memory allocation failed for input buffer\n");
        return BLOCKTYPE2_MALLOC_FAILURE;
    }

    size_t bytes_read = fread(input_buf, 1, input_size, input_file);

    if (!bytes_read || bytes_read != input_size)
    {
        fprintf(stderr, "Error: fread read %zu bytes, expected %zu bytes",
                bytes_read, input_size);
        free(input_buf);
        return BLOCKTYPE2_READ_FAILURE;
    }

    size_t compressed_size = input_size * 2 + 100;
    uint8_t *compressed_buf = malloc(compressed_size);

    if (!compressed_buf)
    {
        fprintf(stderr, "Memory alloaction failed for compressed buffer\n");
        free(input_buf);
        return BLOCKTYPE2_MALLOC_FAILURE;
    }

    LZ77TokenList tokens = {0};

    if (lz77_compress(input_buf, input_size, &tokens) != LZ77_SUCCESS)
    {
        fprintf(stderr, "LZ77 Compression Failed\n");
        free(input_buf);
        return BLOCKTYPE2_LZ77_FAILURE;
    }

    BitWriter bw;

    if (bitwriter_init(&bw, compressed_buf, compressed_size) !=
        BITWRITER_SUCCESS)
    {
        fprintf(stderr, "Error initializing bitwriter\n");
        free(input_buf);
        free(compressed_buf);
        return BLOCKTYPE2_WRITING_FAILURE;
    }

    if (bitwriter_push_bits(&bw, IS_LAST_BIT, IS_LAST_NUM_BITS) !=
        BITWRITER_SUCCESS)
    {
        fprintf(stderr, "Error pushing bits\n");
        free(input_buf);
        free(compressed_buf);
        return BLOCKTYPE2_WRITING_FAILURE;
    }

    if (bitwriter_push_bits(&bw, BTYPE2, BTYPE2_NUM_BITS) != BITWRITER_SUCCESS)
    {
        fprintf(stderr, "Error pushing bits\n");
        free(input_buf);
        free(compressed_buf);
        return BLOCKTYPE2_WRITING_FAILURE;
    }

    uint32_t ll_freqs[286] = {0};
    uint32_t dist_freqs[30] = {0};

    for (size_t i = 0; i < tokens.count; i++)
    {
        if (tokens.tokens[i].type == LITERAL)
        {
            ll_freqs[tokens.tokens[i].literal]++;
        }

        else
        {
            uint16_t len = tokens.tokens[i].LengthDistPair.len;
            uint16_t dist = tokens.tokens[i].LengthDistPair.dist;

            uint16_t ll_code_symbol;
            uint8_t ll_offset_bits;
            uint16_t ll_extra_value;

            length_literal_to_code(len, &ll_code_symbol, &ll_offset_bits, &ll_extra_value);

            uint16_t dist_code_symbol;
            uint8_t dist_offset_bits;
            uint16_t dist_extra_value;

            dist_literal_to_code(dist, &dist_code_symbol, &dist_offset_bits, &dist_extra_value);

            ll_freqs[ll_code_symbol]++;
            dist_freqs[dist_code_symbol]++;
        }
    }
    ll_freqs[256]++; 

    HuffmanDynamicCode *ll_tree = build_huffman_tree(ll_freqs, 286);
    HuffmanDynamicCode *dist_tree = build_huffman_tree(dist_freqs, 30);

    uint8_t ll_lengths[286] = {0};
    uint8_t dist_lengths[30] = {0};

    generate_code_lengths(ll_tree, ll_lengths, 0);
    generate_code_lengths(dist_tree, dist_lengths, 0);

    uint16_t hlit = 286;
    while (hlit > 257 && ll_lengths[hlit - 1] == 0) hlit--;

    uint16_t hdist = 30;
    while (hdist > 1 && dist_lengths[hdist - 1] == 0) hdist--;

    size_t cl_seq_len = hlit + hdist;
    uint8_t *cl_seq = malloc(cl_seq_len);

    if (!cl_seq)
    {
        fprintf(stderr, "Error allocating memory for cl_seq\n");

        free(input_buf);
        free(compressed_buf);

        lz77_free_tokens(&tokens);

        free_huffman_tree(ll_tree);
        free_huffman_tree(dist_tree);

        return BLOCKTYPE2_MALLOC_FAILURE;
    }

    memcpy(cl_seq, ll_lengths, hlit);
    memcpy(cl_seq + hlit, dist_lengths, hdist);

    RLEItem rle_seq[600];
    size_t rle_len;

    rle_encode(cl_seq, cl_seq_len, rle_seq, &rle_len);

    uint32_t cl_freqs[19] = {0};

    for (size_t i = 0; i < rle_len; i++)
    {
        uint8_t sym = rle_seq[i].symbol;

        if (sym < 19)
        {
            cl_freqs[sym]++;
        }
    }

    HuffmanDynamicCode *cl_tree  = build_huffman_tree(cl_freqs, 19);

    uint8_t cl_lengths[19] = {0};
    generate_code_lengths(cl_tree, cl_lengths, 0);

    uint16_t hclen = 19;

    while (hclen > 4 && cl_lengths[CL_ORDER[hclen - 1]] == 0) hclen--;

    bitwriter_push_bits(&bw, hlit - 257, 5);
    bitwriter_push_bits(&bw, hdist - 1, 5);
    bitwriter_push_bits(&bw, hclen - 4, 4);

    for (int i = 0; i < hclen; i++)
    {
        bitwriter_push_bits(&bw, cl_lengths[CL_ORDER[i]], 3);
    }

    uint16_t cl_codes[19];
    build_canonical_codes(cl_lengths, cl_codes, 19);

    for (size_t i = 0; i < rle_len; i++)
    {
        uint8_t sym = rle_seq[i].symbol;
        uint16_t code = cl_codes[sym];
        uint8_t bitlen = cl_lengths[sym];

        bitwriter_push_bits(&bw, code, bitlen);

        if (rle_seq[i].offset_bits > 0)
        {
            bitwriter_push_bits(&bw, rle_seq[i].extra_value, rle_seq[i].offset_bits);
        }
    }

    uint16_t ll_codes[286];
    uint16_t dist_codes[30];

    build_canonical_codes(ll_lengths, ll_codes, hlit);
    build_canonical_codes(dist_lengths, dist_codes, hdist);

    for (size_t i = 0; i < tokens.count; i++)
    {
        if (tokens.tokens[i].type == LITERAL)
        {
            uint8_t symbol = tokens.tokens[i].literal;
            uint16_t code = ll_codes[symbol];
            uint8_t bitlen = ll_lengths[symbol];

            bitwriter_push_bits(&bw, code, bitlen);
        }

        else
        {
            uint16_t len = tokens.tokens[i].LengthDistPair.len;
            uint16_t dist = tokens.tokens[i].LengthDistPair.dist;

            uint16_t ll_code_symbol;
            uint8_t ll_offset_bits;
            uint16_t ll_extra_value;

            length_literal_to_code(len, &ll_code_symbol, &ll_offset_bits, &ll_extra_value);

            uint16_t ll_code = ll_codes[ll_code_symbol];
            uint16_t ll_bitlen = ll_lengths[ll_code_symbol];

            bitwriter_push_bits(&bw, ll_code, ll_bitlen);

            if (ll_offset_bits > 0)
            {
                bitwriter_push_bits(&bw, ll_extra_value, ll_offset_bits);
            }

            uint16_t dist_code_symbol;
            uint8_t dist_offset_bits;
            uint16_t dist_extra_value;

            dist_literal_to_code(dist, &dist_code_symbol, &dist_offset_bits, &dist_extra_value);

            uint16_t dist_code = dist_codes[dist_code_symbol];
            uint16_t dist_bitlen = dist_lengths[dist_code_symbol];

            bitwriter_push_bits(&bw, dist_code, dist_bitlen);

            if (dist_offset_bits > 0)
            {
                bitwriter_push_bits(&bw, dist_extra_value, dist_offset_bits);
            }
        }
    }

    uint16_t eob_code = ll_codes[256];
    uint8_t eob_bitlen = ll_lengths[256];

    bitwriter_push_bits(&bw, eob_code, eob_bitlen);

    size_t bytes_written;
    bitwriter_flush(&bw, &bytes_written);

    fwrite(compressed_buf, 1, bytes_written, output_file);
    // bitwriter_write_bits(&bw, output_file);

    uint32_t crc = update_crc(0, input_buf, (uint32_t)input_size);

    if (write_gzip_footer(output_file, crc, (uint32_t)input_size) != GZIP_FOOTER_SUCCESS)
    {
        fprintf(stderr, "Error writing gzip footer for Block Type 0\n");
        free(input_buf);
        free(compressed_buf);

        lz77_free_tokens(&tokens);

        free_huffman_tree(ll_tree);
        free_huffman_tree(dist_tree);
        free_huffman_tree(cl_tree);

        return BLOCKTYPE2_FOOTER_FAILURE;
    }

    free(input_buf);
    free(compressed_buf);

    lz77_free_tokens(&tokens);

    free_huffman_tree(ll_tree);
    free_huffman_tree(dist_tree);
    free_huffman_tree(cl_tree);

    return BLOCKTYPE2_SUCCESS;
}
