#include "blocktype1.h"

#include "footer.h"
#include "header.h"
#include "huffman_fixed.h"

#include <stdint.h>
#include <stdlib.h>

B1_STATUS write_fixed_huffman_block(BitWriter *bw, 
        const LZ77TokenList *tokens)
{
    BITWRITER_STATUS bw_status;

    for (size_t i = 0; i < tokens->count; i++)
    {
        const LZ77Token *token = &tokens->data[i];

        if (token->type == LZ77_LITERAL)
        {
            HuffmanFixedCode hcode = get_fixed_literal_code(token->literal);

            bw_status = bitwriter_push_bits(bw, hcode.code, hcode.bitlen);

            if (bw_status != BITWRITER_SUCCESS)
            {
                return B1_WRITING_FAILURE;
            }
        }

        else
        {
            uint16_t len = token->match.len;
            uint16_t dist = token->match.dist;

            uint16_t ll_sym, ll_extra;
            uint8_t ll_bits;

            length_literal_to_code(len, &ll_sym, &ll_bits, &ll_extra);

            HuffmanFixedCode ll_code = get_fixed_literal_code(ll_sym);

            bw_status = bitwriter_push_bits(bw, ll_code.code, ll_code.bitlen);

            if (bw_status != BITWRITER_SUCCESS)
            {
                return B1_WRITING_FAILURE;
            }

            if (ll_bits)
            {
                bw_status = bitwriter_push_bits(bw, ll_extra, ll_bits);

                if (bw_status != BITWRITER_SUCCESS)
                {
                    return B1_WRITING_FAILURE;
                }
            }

            uint16_t dist_sym, dist_extra;
            uint8_t dist_bits;

            dist_literal_to_code(dist, &dist_sym, &dist_bits, &dist_extra);

            HuffmanFixedCode dist_code = get_fixed_dist_code(dist_sym);

            bw_status =
                bitwriter_push_bits(bw, dist_code.code, dist_code.bitlen);

            if (bw_status != BITWRITER_SUCCESS)
            {
                return B1_WRITING_FAILURE;
            }

            if (dist_bits)
            {
                bw_status = bitwriter_push_bits(bw, dist_extra, dist_bits);

                if (bw_status != BITWRITER_SUCCESS)
                {
                    return B1_WRITING_FAILURE;
                }
            }
        }
    }

    return B1_SUCCESS;
}

B1_STATUS blocktype1_encoding(FILE *input_file, FILE *output_file)
{

    B1_STATUS status = B1_SUCCESS;

    uint8_t *input_buf = NULL;
    uint8_t *compressed_buf = NULL;

    LZ77TokenList tokens = {0};

    GZIP_HEADER_STATUS header_status = write_gzip_header(output_file);

    if (header_status != GZIP_HEADER_SUCCESS)
    {
        status = B1_HEADER_FAILURE;
        goto cleanup;
    }

    fseek(input_file, 0, SEEK_END);
    size_t input_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    input_buf = malloc(input_size);

    if (!input_buf)
    {
        status = B1_MALLOC_FAILURE;
        goto cleanup;
    }

    size_t bytes_read = fread(input_buf, 1, input_size, input_file);

    if (!bytes_read || bytes_read != input_size)
    {
        status = B1_READ_FAILURE;
        goto cleanup;
    }

    // some extra space to ensure buf big enough
    size_t compressed_size = input_size * 2 + 100;
    compressed_buf = malloc(compressed_size);

    if (!compressed_buf)
    {
        status = B1_MALLOC_FAILURE;
        goto cleanup;
    }

    LZ77_STATUS lz77_status = lz77_compress(input_buf, input_size, &tokens);

    if (lz77_status != LZ77_SUCCESS)
    {
        status = B1_LZ77_FAILURE;
        goto cleanup;
    }

    BitWriter bw;
    BITWRITER_STATUS bw_status =
        bitwriter_init(&bw, compressed_buf, compressed_size);

    if (bw_status != BITWRITER_SUCCESS)
    {
        status = B1_WRITING_FAILURE;
        goto cleanup;
    }

    bw_status = bitwriter_push_bits(&bw, IS_LAST_BIT, IS_LAST_NUM_BITS);

    if (bw_status != BITWRITER_SUCCESS)
    {
        status = B1_WRITING_FAILURE;
        goto cleanup;
    }

    bw_status = bitwriter_push_bits(&bw, BTYPE1, BTYPE1_NUM_BITS);

    if (bw_status != BITWRITER_SUCCESS)
    {
        status = B1_WRITING_FAILURE;
        goto cleanup;
    }

    init_fixed_huffman_tables();
    status = write_fixed_huffman_block(&bw, &tokens);

    if (status != B1_SUCCESS)
    {
        status = B1_WRITING_FAILURE;
        goto cleanup;
    }

    HuffmanFixedCode eob = get_fixed_literal_code(256);
    bw_status = bitwriter_push_bits(&bw, eob.code, eob.bitlen);

    if (bw_status != BITWRITER_SUCCESS)
    {
        status = B1_WRITING_FAILURE;
        goto cleanup;
    }

    size_t bytes_written;

    bw_status = bitwriter_flush(&bw, &bytes_written);

    if (bw_status != BITWRITER_SUCCESS)
    {
        status = B1_WRITING_FAILURE;
        goto cleanup;
    }

    bitwriter_write_bits(&bw, output_file);

    uint32_t crc = update_crc(0, input_buf, (uint32_t)input_size);

    GZIP_FOOTER_STATUS footer_status =
        write_gzip_footer(output_file, crc, (uint32_t)input_size);

    if (footer_status != GZIP_FOOTER_SUCCESS)
    {
        status = B1_FOOTER_FAILURE;
        goto cleanup;
    }

cleanup:
    free(input_buf);
    free(compressed_buf);

    lz77_free_tokens(&tokens);

    return status;
}
