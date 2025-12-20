#include "blocktype2.h"

#include "footer.h"
#include "header.h"
#include "huffman_dynamic.h"
#include "huffman_fixed.h"
#include "rle.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static const uint8_t CL_ORDER[] = {16, 17, 18, 0, 8,  7, 9,  6, 10, 5,
                                   11, 4,  12, 3, 13, 2, 14, 1, 15};

B2_STATUS build_dynamic_huffman_tables(const LZ77TokenList *tokens,
        DynamicHuffmanTables *t)
{
    uint32_t ll_freqs[286] = {0};
    uint32_t dist_freqs[30] = {0};
    uint32_t cl_freqs[19] = {0};

    HuffmanDynamicCode *ll_tree = NULL;
    HuffmanDynamicCode *dist_tree = NULL;
    HuffmanDynamicCode *cl_tree = NULL;

    uint8_t *len_seq = NULL;

    lz77_count_freq(tokens, ll_freqs, dist_freqs);

    ll_tree = build_huffman_tree(ll_freqs, 286);
    dist_tree = build_huffman_tree(dist_freqs, 30);

    if (!ll_tree || !dist_tree)
    {
        goto fail;
    }

    generate_code_lengths(ll_tree, t->ll_lengths, 0);
    generate_code_lengths(dist_tree, t->dist_lengths, 0);

    t->hlit = 286;

    while (t->hlit > 257 && t->ll_lengths[t->hlit - 1] == 0)
    {
        t->hlit--;
    }

    t->hdist = 30;

    while (t->hdist > 1 && t->dist_lengths[t->hdist - 1] == 0)
    {
        t->hdist--;
    }

    size_t len_seq_len = t->hlit + t->hdist;
    len_seq = malloc(len_seq_len);

    if (!len_seq)
    {
        goto fail;
    }

    memcpy(len_seq, t->ll_lengths, t->hlit);
    memcpy(len_seq + t->hlit, t->dist_lengths, t->hdist);

    t->rle_seq = malloc(len_seq_len * 2 * sizeof(RLEItem));

    if (!t->rle_seq)
    {
        goto fail;
    }

    rle_encode(len_seq, len_seq_len, t->rle_seq, &t->rle_len);

    for (size_t i = 0; i < t->rle_len; i++)
    {
        uint8_t sym = t->rle_seq[i].symbol;
        if (sym < 19)
            cl_freqs[sym]++;
    }

    cl_tree = build_huffman_tree(cl_freqs, 19);

    if (!cl_tree)
    {
        goto fail;
    }

    generate_code_lengths(cl_tree, t->cl_lengths, 0);

    t->hclen = 19;

    while (t->hclen > 4 && t->cl_lengths[CL_ORDER[t->hclen - 1]] == 0)
    {
        t->hclen--;
    }

    build_canonical_codes(t->ll_lengths, t->ll_codes, 286);
    build_canonical_codes(t->dist_lengths, t->dist_codes, 30);
    build_canonical_codes(t->cl_lengths, t->cl_codes, 19);

    free(len_seq);

    free_huffman_tree(ll_tree);
    free_huffman_tree(dist_tree);
    free_huffman_tree(cl_tree);

    return B2_SUCCESS;

fail:
    free(len_seq);
    free(t->rle_seq);

    free_huffman_tree(ll_tree);
    free_huffman_tree(dist_tree);
    free_huffman_tree(cl_tree);

    return B2_MALLOC_FAILURE;
}

B2_STATUS emit_dynamic_huffman_tokens(BitWriter *bw, const LZ77TokenList *tokens,
        const HuffmanEmitTables *t)
{
    BITWRITER_STATUS bw_status;

    for (size_t i = 0; i < tokens->count; i++)
    {
        const LZ77Token *token = &tokens->data[i];

        if (token->type == LZ77_LITERAL)
        {
            uint8_t sym = token->literal;

            bw_status =
                bitwriter_push_bits(bw, t->ll_codes[sym], t->ll_lengths[sym]);

            if (bw_status != BITWRITER_SUCCESS)
                return B2_WRITING_FAILURE;
        }

        else
        {
            uint16_t ll_sym, ll_extra;
            uint8_t ll_bits;

            length_literal_to_code(token->match.len, &ll_sym, &ll_bits,
                                   &ll_extra);

            bw_status = bitwriter_push_bits(bw, t->ll_codes[ll_sym],
                                            t->ll_lengths[ll_sym]);

            if (bw_status != BITWRITER_SUCCESS)
                return B2_WRITING_FAILURE;

            if (ll_bits)
            {
                bw_status = bitwriter_push_bits(bw, ll_extra, ll_bits);
                if (bw_status != BITWRITER_SUCCESS)
                    return B2_WRITING_FAILURE;
            }

            uint16_t dist_sym, dist_extra;
            uint8_t dist_bits;

            dist_literal_to_code(token->match.dist, &dist_sym, &dist_bits,
                                 &dist_extra);

            bw_status = bitwriter_push_bits(bw, t->dist_codes[dist_sym],
                                            t->dist_lengths[dist_sym]);

            if (bw_status != BITWRITER_SUCCESS)
                return B2_WRITING_FAILURE;

            if (dist_bits)
            {
                bw_status = bitwriter_push_bits(bw, dist_extra, dist_bits);
                if (bw_status != BITWRITER_SUCCESS)
                    return B2_WRITING_FAILURE;
            }
        }
    }

    return B2_SUCCESS;
}

B2_STATUS emit_dynamic_huffman_header(BitWriter *bw, DynamicHuffmanTables *t) {
    BITWRITER_STATUS bw_status;

    bw_status = bitwriter_push_bits(bw, t->hlit - 257, 5);
    if (bw_status != BITWRITER_SUCCESS) 
    {
        return B2_WRITING_FAILURE;
    }

    bw_status = bitwriter_push_bits(bw, t->hdist - 1, 5);
    if (bw_status != BITWRITER_SUCCESS) 
    {
        return B2_WRITING_FAILURE;
    }

    bw_status = bitwriter_push_bits(bw, t->hclen - 4, 4);
    if (bw_status != BITWRITER_SUCCESS) 
    {
        return B2_WRITING_FAILURE;
    }

    for (int i = 0; i < t->hclen; i++) {
        bw_status = bitwriter_push_bits(bw, t->cl_lengths[CL_ORDER[i]], 3);
        if (bw_status != BITWRITER_SUCCESS) 
        {
            return B2_WRITING_FAILURE;
        }
    }

    uint16_t cl_codes[19];
    build_canonical_codes(t->cl_lengths, cl_codes, 19);

    for (size_t i = 0; i < t->rle_len; i++) {
        uint8_t sym = t->rle_seq[i].symbol;
        uint16_t code = cl_codes[sym];
        uint8_t bitlen = t->cl_lengths[sym];

        bw_status = bitwriter_push_bits(bw, code, bitlen);
        if (bw_status != BITWRITER_SUCCESS) 
        {
            return B2_WRITING_FAILURE;
        }

        if (t->rle_seq[i].offset_bits > 0) {
            bw_status = bitwriter_push_bits(bw,
                                            t->rle_seq[i].extra_value,
                                            t->rle_seq[i].offset_bits);
            if (bw_status != BITWRITER_SUCCESS) 
            {
                return B2_WRITING_FAILURE;
            }
        }
    }

    return B2_SUCCESS;
}

B2_STATUS blocktype2_encoding(FILE *input_file, FILE *output_file)
{

    B2_STATUS status = B2_SUCCESS;
    uint8_t *input_buf = NULL;
    uint8_t *compressed_buf = NULL;

    LZ77TokenList tokens = {0};
    DynamicHuffmanTables tables = {0};

    GZIP_HEADER_STATUS header_status = write_gzip_header(output_file);

    if (header_status != GZIP_HEADER_SUCCESS)
    {
        return B2_HEADER_FAILURE;
    }

    fseek(input_file, 0, SEEK_END);
    size_t input_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    input_buf = malloc(input_size);

    if (!input_buf)
    {
        status = B2_MALLOC_FAILURE;
        goto cleanup;
    }

    size_t bytes_read = fread(input_buf, 1, input_size, input_file);

    if (!bytes_read || bytes_read != input_size)
    {
        status = B2_READ_FAILURE;
        goto cleanup;
    }

    size_t compressed_size = input_size * 2 + 100;
    compressed_buf = malloc(compressed_size);

    if (!compressed_buf)
    {
        status = B2_MALLOC_FAILURE;
        goto cleanup;
    }

    LZ77_STATUS lz77_status = lz77_compress(input_buf, input_size, &tokens);

    if (lz77_status != LZ77_SUCCESS)
    {
        status = B2_LZ77_FAILURE;
        goto cleanup;
    }

    BitWriter bw;
    BITWRITER_STATUS bw_status =
        bitwriter_init(&bw, compressed_buf, compressed_size);

    if (bw_status != BITWRITER_SUCCESS)
    {
        status = B2_WRITING_FAILURE;
        goto cleanup;
    }

    bw_status = bitwriter_push_bits(&bw, IS_LAST_BIT, IS_LAST_NUM_BITS);

    if (bw_status != BITWRITER_SUCCESS)
    {
        status = B2_WRITING_FAILURE;
        goto cleanup;
    }

    bw_status = bitwriter_push_bits(&bw, BTYPE2, BTYPE2_NUM_BITS);

    if (bw_status != BITWRITER_SUCCESS)
    {
        status = B2_WRITING_FAILURE;
        goto cleanup;
    }

    status = build_dynamic_huffman_tables(&tokens, &tables);

    if (status != B2_SUCCESS)
    {
        goto cleanup;
    }

    status = emit_dynamic_huffman_header(&bw, &tables);


    if (status != B2_SUCCESS)
    {
        goto cleanup;
    }

    HuffmanEmitTables emit_tables = {.ll_codes = tables.ll_codes,
                                     .ll_lengths = tables.ll_lengths,
                                     .dist_codes = tables.dist_codes,
                                     .dist_lengths = tables.dist_lengths};

    status = emit_dynamic_huffman_tokens(&bw, &tokens, &emit_tables);
    if (status != B2_SUCCESS)
        goto cleanup;

    uint16_t eob_code = tables.ll_codes[256];
    uint8_t eob_bitlen = tables.ll_lengths[256];

    bw_status = bitwriter_push_bits(&bw, eob_code, eob_bitlen);

    if (bw_status != BITWRITER_SUCCESS)
    {
        status = B2_WRITING_FAILURE;
        goto cleanup;
    }

    size_t bytes_written;
    bw_status = bitwriter_flush(&bw, &bytes_written);

    if (bw_status != BITWRITER_SUCCESS)
    {
        status = B2_WRITING_FAILURE;
        goto cleanup;
    }

    bw_status = bitwriter_write_bits(&bw, output_file);

    if (bw_status != BITWRITER_SUCCESS)
    {
        status = B2_WRITING_FAILURE;
        goto cleanup;
    }

    uint32_t crc = update_crc(0, input_buf, (uint32_t)input_size);

    GZIP_FOOTER_STATUS footer_status =
        write_gzip_footer(output_file, crc, (uint32_t)input_size);

    if (footer_status != GZIP_FOOTER_SUCCESS)
    {
        status = B2_FOOTER_FAILURE;
        goto cleanup;
    }

cleanup:
    free(input_buf);
    free(compressed_buf);

    lz77_free_tokens(&tokens);
    free(tables.rle_seq);

    return status;
}

static B2_STATUS grow_output_buffer(uint8_t **buf, size_t *capacity)
{
    size_t new_cap = (*capacity) * 2;

    uint8_t *new_buf = realloc(*buf, new_cap);
    if (!new_buf)
    {
        return B2_MALLOC_FAILURE;
    }

    *buf = new_buf;
    *capacity = new_cap;
    return B2_SUCCESS;
}

B2_STATUS blocktype2_decoding(FILE *input_file, FILE *output_file)
{
    B2_STATUS status = B2_SUCCESS;

    uint8_t *input_buf = NULL;
    uint8_t *output_data = NULL;

    HuffmanDynamicCode *ll_tree = NULL;
    HuffmanDynamicCode *dist_tree = NULL;
    HuffmanDynamicCode *cl_tree = NULL;

    GZIP_HEADER_STATUS header_status = read_gzip_header(input_file); 

    if (header_status != GZIP_HEADER_SUCCESS)
    {
        status = B2_HEADER_FAILURE;
        goto cleanup;
    }

    fseek(input_file, 0, SEEK_END);
    size_t file_size = ftell(input_file);
    fseek(input_file, 10, SEEK_SET);

    size_t compressed_size = file_size - 10 - 8;

    input_buf = malloc(compressed_size);

    if (!input_buf)
    {
        status = B2_MALLOC_FAILURE;
        goto cleanup;
    }

    size_t bytes_read = fread(input_buf, 1, compressed_size, input_file);

    if (bytes_read != compressed_size)
    {
        status = B2_READ_FAILURE;
        goto cleanup;
    }

    BitReader br;
    BITREADER_STATUS br_status = bitreader_init(&br, input_buf, compressed_size);

    if (br_status!= BITREADER_SUCCESS)
    {
        free(input_buf);
        return B2_READ_FAILURE;
    }

    uint32_t bfinal = 0, btype = 0;

    br_status = bitreader_read_bits(&br, &bfinal, 1);

    if (br_status != BITREADER_SUCCESS)
    {
        status = B2_READ_FAILURE;
        goto cleanup;
    }

    br_status = bitreader_read_bits(&br, &btype, 2);

    if (br_status != BITREADER_SUCCESS)
    {
        status = B2_READ_FAILURE;
        goto cleanup;
    }

    if (btype != BTYPE2)
    {
        status = B2_INVALID_BLOCKTYPE;
        goto cleanup;
    }

    uint32_t HLIT = 0, HDIST = 0, HCLEN = 0;

    br_status = bitreader_read_bits(&br, &HLIT, 5);   HLIT += 257;

    if (br_status != BITREADER_SUCCESS)
    {
        status = B2_READ_FAILURE;
        goto cleanup;
    }

    br_status = bitreader_read_bits(&br, &HDIST, 5);  HDIST += 1;

    if (br_status != BITREADER_SUCCESS)
    {
        status = B2_READ_FAILURE;
        goto cleanup;
    }

    br_status = bitreader_read_bits(&br, &HCLEN, 4);  HCLEN += 4;

    if (br_status != BITREADER_SUCCESS)
    {
        status = B2_READ_FAILURE;
        goto cleanup;
    }

    uint8_t cl_lengths[19] = {0};

    for (uint32_t i = 0; i < HCLEN; i++)
    {
        uint32_t tmp = 0;
        bitreader_read_bits(&br, &tmp, 3);
        cl_lengths[CL_ORDER[i]] = (uint8_t)tmp;
    }

    cl_tree = build_huffman_tree_from_lengths(cl_lengths, 19);

    if (!cl_tree)
    {
        status = B2_DECODE_FAILURE;
        goto cleanup;
    }

    uint8_t ll_lengths[286] = {0};
    uint8_t dist_lengths[30] = {0};

    uint32_t idx = 0;
    uint32_t total = HLIT + HDIST;

    while (idx < total)
    {
        uint32_t sym = decode_dynamic_literal_or_length(&br, cl_tree);

        if (sym <= 15)
        {
            if (idx < HLIT)
            {
                ll_lengths[idx++] = (uint8_t)sym;
            }

            else
            {
                dist_lengths[idx++ - HLIT] = (uint8_t)sym;
            }
        }
        else if (sym == 16)
        {
            uint32_t extra = 0;
            br_status = bitreader_read_bits(&br, &extra, 2);

            if (br_status != BITREADER_SUCCESS)
            {
                status = B2_READ_FAILURE;
                goto cleanup;
            }

            uint32_t repeat = 3 + extra;
            uint8_t prev = 0;

            if (idx > 0)
            {
                if (idx <= HLIT)
                {
                    prev = ll_lengths[idx - 1];
                }

                else
                {
                    prev = dist_lengths[idx - HLIT - 1];
                }
            }

            while (repeat-- && idx < total)
            {
                if (idx < HLIT)
                {
                    ll_lengths[idx++] = prev;
                }

                else
                {
                    dist_lengths[idx++ - HLIT] = prev;
                }
            }
        }

        else if (sym == 17)
        {
            uint32_t extra = 0;
            br_status = bitreader_read_bits(&br, &extra, 3);

            if (br_status != BITREADER_SUCCESS)
            {
                status = B2_READ_FAILURE;
                goto cleanup;
            }

            uint32_t repeat = 3 + extra;

            while (repeat-- && idx < total)
            {
                if (idx < HLIT)
                {
                    ll_lengths[idx++] = 0;
                }

                else
                {
                    dist_lengths[idx++ - HLIT] = 0;
                }
            }
        }
        else if (sym == 18)
        {
            uint32_t extra = 0;
            br_status = bitreader_read_bits(&br, &extra, 7);

            if (br_status != BITREADER_SUCCESS)
            {
                status = B2_READ_FAILURE;
                goto cleanup;
            }

            uint32_t repeat = 11 + extra;

            while (repeat-- && idx < total)
            {
                if (idx < HLIT)
                {
                    ll_lengths[idx++] = 0;
                }

                else
                {
                    dist_lengths[idx++ - HLIT] = 0;
                }
            }
        }

        else
        {
            status = B2_DECODE_FAILURE;
            goto cleanup;
        }
    }

    ll_tree = build_huffman_tree_from_lengths(ll_lengths, HLIT);
    dist_tree = build_huffman_tree_from_lengths(dist_lengths, HDIST);

    if (!ll_tree || !dist_tree)
    {
        status = B2_DECODE_FAILURE;
        goto cleanup;
    }

    size_t output_capacity = 1024;
    size_t out_size = 0;
    output_data = malloc(output_capacity);

    if (!output_data)
    {
        status = B2_MALLOC_FAILURE;
        goto cleanup;
    }

    while (1)
    {
        uint32_t sym;
        B2_STATUS status;

        status = decode_dynamic_literal_or_length_b2(&br, ll_tree, &sym);
        if (status != B2_SUCCESS)
        {
            goto cleanup;
        }

        if (sym < 256)
        {
            if (out_size >= output_capacity)
                grow_output_buffer(&output_data, &output_capacity);

            output_data[out_size++] = (uint8_t)sym;
        }

        else if (sym == 256)
        {
            break;
        }

        else
        {
            uint16_t length = decode_length_symbol((uint16_t)sym, &br);
            if (length == 0)
            {
                status = B2_DECODE_FAILURE;
                goto cleanup;
            }

            uint16_t dist_sym;
            status = decode_dynamic_distance_symbol_b2(&br, dist_tree, &dist_sym);
            if (status != B2_SUCCESS)
            {
                goto cleanup;
            }

            uint16_t distance = decode_distance_symbol(dist_sym, &br);
            if (distance == 0 || distance > out_size)
            {
                status = B2_DECODE_FAILURE;
                goto cleanup;
            }

            for (uint16_t i = 0; i < length; i++)
            {
                if (out_size >= output_capacity)
                    grow_output_buffer(&output_data, &output_capacity);

                output_data[out_size] =
                    output_data[out_size - distance];
                out_size++;
            }
        }
    }

    fwrite(output_data, 1, out_size, output_file);

cleanup:
    free(input_buf);
    free(output_data);

    free_huffman_tree(cl_tree);
    free_huffman_tree(ll_tree);
    free_huffman_tree(dist_tree);

    return status;
}

B2_STATUS gunzip_decoder(FILE *input_file, FILE *output_file)
{
    B2_STATUS status = B2_SUCCESS;

    uint8_t *input_buf = NULL;
    uint8_t *output_data = NULL;

    HuffmanDynamicCode *ll_tree = NULL;
    HuffmanDynamicCode *dist_tree = NULL;
    HuffmanDynamicCode *cl_tree = NULL;

    GZIP_HEADER_STATUS header_status =
        read_gzip_header_true(input_file);

    if (header_status != GZIP_HEADER_SUCCESS)
        return B2_HEADER_FAILURE;

    long deflate_start = ftell(input_file);
    if (deflate_start < 0)
    {
        status = B2_READ_FAILURE;
        goto cleanup;
    }

    if (fseek(input_file, 0, SEEK_END) != 0)
    {
        status = B2_READ_FAILURE;
        goto cleanup;
    }

    long file_size = ftell(input_file);
    if (file_size < 0)
    {
        status = B2_READ_FAILURE;
        goto cleanup;
    }

    if (file_size < deflate_start + 8)
    {
        status = B2_READ_FAILURE;
        goto cleanup;
    }

    size_t compressed_size = (size_t)(file_size - deflate_start - 8);

    if (fseek(input_file, deflate_start, SEEK_SET) != 0)
    {
        status = B2_READ_FAILURE;
        goto cleanup;
    }

    input_buf = malloc(compressed_size);
    if (!input_buf)
    {
        status = B2_MALLOC_FAILURE;
        goto cleanup;
    }

    if (fread(input_buf, 1, compressed_size, input_file) != compressed_size)
    {
        status = B2_READ_FAILURE;
        goto cleanup;
    }

    BitReader br;
    if (bitreader_init(&br, input_buf, compressed_size) != BITREADER_SUCCESS)
    {
        status = B2_READ_FAILURE;
        goto cleanup;
    }

    size_t output_capacity = 1024;
    size_t out_size = 0;
    output_data = malloc(output_capacity);

    if (!output_data)
    {
        status = B2_MALLOC_FAILURE;
        goto cleanup;
    }

    int last_block = 0;

    while (!last_block)
    {
        uint32_t bfinal = 0, btype = 0;

        if (bitreader_read_bits(&br, &bfinal, 1) != BITREADER_SUCCESS ||
            bitreader_read_bits(&br, &btype, 2) != BITREADER_SUCCESS)
        {
            status = B2_READ_FAILURE;
            goto cleanup;
        }

        last_block = (int)bfinal;

        if (btype != BTYPE2)
        {
            status = B2_INVALID_BLOCKTYPE;
            goto cleanup;
        }

        uint32_t tmp;
        uint16_t HLIT, HDIST, HCLEN;

        if (bitreader_read_bits(&br, &tmp, 5) != BITREADER_SUCCESS) goto cleanup;
        HLIT = (uint16_t)(tmp + 257);

        if (bitreader_read_bits(&br, &tmp, 5) != BITREADER_SUCCESS) goto cleanup;
        HDIST = (uint16_t)(tmp + 1);

        if (bitreader_read_bits(&br, &tmp, 4) != BITREADER_SUCCESS) goto cleanup;
        HCLEN = (uint16_t)(tmp + 4);

        static const uint8_t cl_order[19] =
            {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};

        uint8_t cl_lengths[19] = {0};

        for (uint16_t i = 0; i < HCLEN; i++)
        {
            if (bitreader_read_bits(&br, &tmp, 3) != BITREADER_SUCCESS)
                goto cleanup;
            cl_lengths[cl_order[i]] = (uint8_t)tmp;
        }

        cl_tree = build_huffman_tree_from_lengths(cl_lengths, 19);
        if (!cl_tree)
        {
            status = B2_DECODE_FAILURE;
            goto cleanup;
        }

        uint8_t ll_lengths[286] = {0};
        uint8_t dist_lengths[30] = {0};

        uint16_t idx = 0;
        uint16_t total = (uint16_t)(HLIT + HDIST);

        while (idx < total)
        {
            uint32_t sym = decode_dynamic_literal_or_length(&br, cl_tree);

            if (sym <= 15)
            {
                if (idx < HLIT)
                    ll_lengths[idx++] = (uint8_t)sym;
                else
                    dist_lengths[idx++ - HLIT] = (uint8_t)sym;
            }
            else if (sym == 16)
            {
                uint32_t rep;
                if (bitreader_read_bits(&br, &rep, 2) != BITREADER_SUCCESS)
                    goto cleanup;

                uint8_t val = (idx == 0) ? 0 :
                    (idx <= HLIT ? ll_lengths[idx - 1]
                                 : dist_lengths[idx - HLIT - 1]);

                for (uint32_t j = 0; j < rep + 3 && idx < total; j++)
                {
                    if (idx < HLIT)
                        ll_lengths[idx++] = val;
                    else
                        dist_lengths[idx++ - HLIT] = val;
                }
            }
            else if (sym == 17)
            {
                uint32_t rep;
                if (bitreader_read_bits(&br, &rep, 3) != BITREADER_SUCCESS)
                    goto cleanup;

                for (uint32_t j = 0; j < rep + 3 && idx < total; j++)
                {
                    if (idx < HLIT)
                        ll_lengths[idx++] = 0;
                    else
                        dist_lengths[idx++ - HLIT] = 0;
                }
            }
            else if (sym == 18)
            {
                uint32_t rep;
                if (bitreader_read_bits(&br, &rep, 7) != BITREADER_SUCCESS)
                    goto cleanup;

                for (uint32_t j = 0; j < rep + 11 && idx < total; j++)
                {
                    if (idx < HLIT)
                        ll_lengths[idx++] = 0;
                    else
                        dist_lengths[idx++ - HLIT] = 0;
                }
            }
            else
            {
                status = B2_DECODE_FAILURE;
                goto cleanup;
            }
        }

        free_huffman_tree(cl_tree);
        cl_tree = NULL;

        ll_tree = build_huffman_tree_from_lengths(ll_lengths, HLIT);
        dist_tree = build_huffman_tree_from_lengths(dist_lengths, HDIST);

        if (!ll_tree || !dist_tree)
        {
            status = B2_DECODE_FAILURE;
            goto cleanup;
        }

        for (;;)
        {
            uint32_t sym = decode_dynamic_literal_or_length(&br, ll_tree);

            if (sym < 256)
            {
                if (out_size >= output_capacity)
                {
                    output_capacity *= 2;
                    uint8_t *tmp_out = realloc(output_data, output_capacity);
                    if (!tmp_out)
                    {
                        status = B2_MALLOC_FAILURE;
                        goto cleanup;
                    }
                    output_data = tmp_out;
                }
                output_data[out_size++] = (uint8_t)sym;
            }
            else if (sym == 256)
            {
                break;
            }
            else
            {
                uint16_t length = decode_length_symbol(sym, &br);
                uint32_t dist_sym =
                    decode_dynamic_distance_symbol(&br, dist_tree);
                uint16_t distance =
                    decode_distance_symbol(dist_sym, &br);

                if (distance == 0 || distance > out_size)
                {
                    status = B2_DECODE_FAILURE;
                    goto cleanup;
                }

                for (uint16_t i = 0; i < length; i++)
                {
                    if (out_size >= output_capacity)
                    {
                        output_capacity *= 2;
                        uint8_t *tmp_out = realloc(output_data, output_capacity);
                        if (!tmp_out)
                        {
                            status = B2_MALLOC_FAILURE;
                            goto cleanup;
                        }
                        output_data = tmp_out;
                    }

                    output_data[out_size] =
                        output_data[out_size - distance];
                    out_size++;
                }
            }
        }

        free_huffman_tree(ll_tree);
        free_huffman_tree(dist_tree);
        ll_tree = dist_tree = NULL;
    }

    uint32_t stored_crc = 0, stored_isize = 0;

    if (fseek(input_file, file_size - 8, SEEK_SET) != 0)
        goto cleanup;

    if (fread(&stored_crc, 4, 1, input_file) != 1 ||
        fread(&stored_isize, 4, 1, input_file) != 1)
        goto cleanup;

    uint32_t calc_crc = update_crc(0, output_data, out_size);

    if (stored_crc != calc_crc ||
        stored_isize != (out_size & 0xFFFFFFFF))
    {
        status = B2_DECODE_FAILURE;
        goto cleanup;
    }

    fwrite(output_data, 1, out_size, output_file);

cleanup:
    free(input_buf);
    free(output_data);
    free_huffman_tree(cl_tree);
    free_huffman_tree(ll_tree);
    free_huffman_tree(dist_tree);
    return status;
}
