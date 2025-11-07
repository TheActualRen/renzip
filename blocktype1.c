#include "blocktype1.h"
#include "header.h"
#include "lz77.h"
#include "huffman_fixed.h"
#include "bitwriter.h"
#include "footer.h"

#include <stdint.h>
#include <stdlib.h>

int blocktype1_encoding(FILE *input_file, FILE *output_file)
{

    if (write_gzip_header(output_file) != GZIP_HEADER_SUCCESS)
    {
        fprintf(stderr, "Error writing gzip header, Block Type 0\n");
        return BLOCKTYPE1_HEADER_FAILURE;
    }

    fseek(input_file, 0, SEEK_END);
    size_t input_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);


    uint8_t *input_buf = malloc(input_size);

    if (!input_buf)
    {
        fprintf(stderr, "Memory allocation failed for input buffer\n");
        return BLOCKTYPE1_MALLOC_FAILURE;
    }

    size_t bytes_read = fread(input_buf, 1, input_size, input_file);

    if (!bytes_read || bytes_read != input_size)
    {
        fprintf(stderr, "Error: fread read %zu bytes, expected %zu bytes",
                bytes_read, input_size);
        free(input_buf);
        return BLOCKTYPE1_READ_FAILURE;
    }

    size_t compressed_size = input_size * 2 + 100;
    uint8_t *compressed_buf = malloc(compressed_size);

    if (!compressed_buf)
    {
        fprintf(stderr, "Memory alloaction failed for compressed buffer\n");
        free(input_buf);
        return BLOCKTYPE1_MALLOC_FAILURE;
    }

    LZ77TokenList tokens = {0};

    if (lz77_compress(input_buf, input_size, &tokens) != LZ77_SUCCESS)
    {
        fprintf(stderr, "LZ77 Compression Failed\n");
        free(input_buf);
        return BLOCKTYPE1_LZ77_FAILURE;
    }

    BitWriter bw;

    if (bitwriter_init(&bw, compressed_buf, compressed_size) !=
        BITWRITER_SUCCESS)
    {
        fprintf(stderr, "Error initializing bitwriter\n");
        free(input_buf);
        free(compressed_buf);
        return BLOCKTYPE1_WRITING_FAILURE;
    }

    if (bitwriter_push_bits(&bw, IS_LAST_BIT, IS_LAST_NUM_BITS) !=
        BITWRITER_SUCCESS)
    {
        fprintf(stderr, "Error pushing bits\n");
        free(input_buf);
        free(compressed_buf);
        return BLOCKTYPE1_WRITING_FAILURE;
    }

    if (bitwriter_push_bits(&bw, BTYPE1, BTYPE1_NUM_BITS) != BITWRITER_SUCCESS)
    {
        fprintf(stderr, "Error pushing bits\n");
        free(input_buf);
        free(compressed_buf);
        return BLOCKTYPE1_WRITING_FAILURE;
    }

    init_fixed_huffman_tables();

    for (size_t i = 0; i < tokens.count; i++)
    {
        if (tokens.tokens[i].type == LITERAL)
        {
            HuffmanFixedCode hcode = get_fixed_literal_code(tokens.tokens[i].literal);
            bitwriter_push_bits(&bw, hcode.code, hcode.bitlen);
        }

        else if (tokens.tokens[i].type == LENGTH_DIST_PAIR)
        {
            uint16_t len = tokens.tokens[i].LengthDistPair.len;
            uint16_t dist = tokens.tokens[i].LengthDistPair.dist;

            uint16_t ll_code_symbol;
            uint8_t ll_offset_bits;
            uint16_t ll_extra_value;

            length_literal_to_code(len, &ll_code_symbol, &ll_offset_bits, &ll_extra_value);

            HuffmanFixedCode ll_code = get_fixed_literal_code(ll_code_symbol);
            bitwriter_push_bits(&bw, ll_code.code, ll_code.bitlen);

            if (ll_offset_bits > 0)
            {
                if (bitwriter_push_bits(&bw, ll_extra_value, ll_offset_bits) != BITWRITER_SUCCESS)
                {
                    fprintf(stderr, "Error: BitWriter buffer overflow\n");
                    free(input_buf);
                    free(compressed_buf);
                    return BLOCKTYPE1_WRITING_FAILURE;
                }
            }

            uint16_t dist_code_symbol;
            uint8_t dist_offset_bits;
            uint16_t dist_extra_value;

            dist_literal_to_code(dist, &dist_code_symbol, &dist_offset_bits, &dist_extra_value);

            HuffmanFixedCode dist_code = get_fixed_dist_code(dist_code_symbol);
            bitwriter_push_bits(&bw, dist_code.code, dist_code.bitlen);

            if (dist_offset_bits > 0)
            {
                bitwriter_push_bits(&bw, dist_extra_value, dist_offset_bits);
            }
        }
    }

    HuffmanFixedCode eob = get_fixed_literal_code(256);
    bitwriter_push_bits(&bw, eob.code, eob.bitlen);

    size_t bytes_written;
    bitwriter_flush(&bw, &bytes_written);

    // bitwriter_write_bits(&bw, output_file);
    fwrite(compressed_buf, 1, bytes_written, output_file);

    lz77_free_tokens(&tokens);

    uint32_t crc = update_crc(0, input_buf, (uint32_t)input_size);

    if (write_gzip_footer(output_file, crc, (uint32_t)input_size) != GZIP_FOOTER_SUCCESS)
    {
        fprintf(stderr, "Error writing gzip footer for Block Type 0\n");
        free(input_buf);
        free(compressed_buf);
        return BLOCKTYPE1_FOOTER_FAILURE;
    }

    free(input_buf);
    free(compressed_buf);

    return BLOCKTYPE1_SUCCESS;
}
