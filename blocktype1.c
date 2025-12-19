#include "blocktype1.h"

#include "footer.h"
#include "header.h"
#include "huffman_fixed.h"
#include "bitreader.h"

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
HuffmanFixedCode dist_code = get_fixed_dist_code(dist_sym); bw_status =
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

static B1_STATUS grow_output_buffer(uint8_t **buf, size_t *capacity)
{
    size_t new_cap = (*capacity) * 2;

    uint8_t *new_buf = realloc(*buf, new_cap);
    if (!new_buf)
    {
        return B1_MALLOC_FAILURE;
    }

    *buf = new_buf;
    *capacity = new_cap;
    return B1_SUCCESS;
}

B1_STATUS blocktype1_decoding(FILE *input_file, FILE *output_file)
{
    B1_STATUS status = B1_SUCCESS;

    uint8_t *input_buf = NULL;
    uint8_t *output_data = NULL;

    GZIP_HEADER_STATUS header_status = read_gzip_header(input_file);

    if (header_status != GZIP_HEADER_SUCCESS)
    {
        status = B1_HEADER_FAILURE;
        goto cleanup;
    }

    fseek(input_file, 0, SEEK_END);
    size_t file_size = ftell(input_file);
    fseek(input_file, 10, SEEK_SET);

    size_t compressed_size = file_size - 10 - 8;

    input_buf = malloc(compressed_size);

    if (!input_buf)
    {
        status = B1_MALLOC_FAILURE;
        goto cleanup;
    }

    size_t bytes_read = fread(input_buf, 1, compressed_size, input_file);

    if (bytes_read != compressed_size)
    {
        status = B1_READ_FAILURE;
        goto cleanup;
    }

    BitReader br;
    BITREADER_STATUS br_status = bitreader_init(&br, input_buf, compressed_size);

    if (br_status != BITREADER_SUCCESS)
    {
        status = B1_READ_FAILURE;
        goto cleanup;
    }

    uint32_t bfinal, btype;
    br_status = bitreader_read_bits(&br, &bfinal, IS_LAST_NUM_BITS);

    if (br_status != BITREADER_SUCCESS)
    {
        status = B1_READ_FAILURE;
        goto cleanup;
    }

    br_status = bitreader_read_bits(&br, &btype, BTYPE1_NUM_BITS);

    if (br_status != BITREADER_SUCCESS)
    {
        status = B1_READ_FAILURE;
        goto cleanup;
    }

    if (btype != BTYPE1)
    {
        status = B1_INVALID_BLOCKTYPE;
        goto cleanup;
    }

    init_fixed_huffman_tables();

    size_t output_capacity = 1024;
    size_t out_size = 0;
    output_data = malloc(output_capacity);

    if (!output_data)
    {
        status = B1_MALLOC_FAILURE;
        goto cleanup;
    }
   

    while (1)
    {
        uint16_t sym;

        status = decode_fixed_literal_or_length(&br, &sym);
        if (status != B1_SUCCESS)
        {
            goto cleanup;
        }

        if (sym < 256)
        {
            if (out_size >= output_capacity)
            {
                status = grow_output_buffer(&output_data, &output_capacity);

                if (status != B1_SUCCESS)
                {
                    goto cleanup;
                }
            }

            output_data[out_size++] = (uint8_t)sym;
        }

        else if (sym == 256)
        {
            break;
        }

        else if (sym >= 257 && sym <= 285)
        {
            uint16_t length;
            status = decode_length_symbol(sym, &br, &length);

            if (status != B1_SUCCESS)
            {
                goto cleanup;
            }

            uint16_t dist_sym;
            status = decode_fixed_distance_symbol(&br, &dist_sym);

            if (status != B1_SUCCESS)
            {
                goto cleanup;
            }


            uint16_t distance;

            status = decode_distance_symbol(dist_sym, &br, &distance);
            if (status != B1_SUCCESS)
            {
                goto cleanup;
            }

            if (distance == 0 || distance > out_size)
            {
                status = B1_DECODE_FAILURE;
                goto cleanup;
            }

            for (uint16_t i = 0; i < length; i++)
            {
                uint8_t byte = output_data[out_size - distance];

                if (out_size >= output_capacity)
                {
                    status = grow_output_buffer(&output_data, &output_capacity);

                    if (status != B1_SUCCESS)
                    {
                        goto cleanup;
                    }
                }

                output_data[out_size++] = byte;
            }
        }

        else
        {
            status = B1_DECODE_FAILURE;
            goto cleanup;
        }
    }

    uint32_t stored_crc, stored_isize;

    if (fread(&stored_crc, 4, 1, input_file) != 1)
    {
        return B1_READ_FAILURE;
    }


    if (fread(&stored_isize, 4, 1, input_file) != 1)
    {
        status = B1_READ_FAILURE;
        goto cleanup;
    }

    uint32_t valid_crc = update_crc(0, output_data, out_size);

    if (stored_crc != valid_crc)
    {
        status = B1_FOOTER_FAILURE;
        goto cleanup;
    }

    if (stored_isize != (out_size & 0xFFFFFFFF))
    {
        status = B1_FOOTER_FAILURE;
        goto cleanup;
    }

    fwrite(output_data, 1, out_size, output_file);

cleanup:
    free(input_buf);
    free(output_data);

    return status;
}
