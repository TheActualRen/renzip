#include "huffman_fixed.h"
#include "tables/fixed_huffman_tables.h"


static HuffmanFixedCode ll_codes[NUM_LL_CODES];
static HuffmanFixedCode dist_codes[NUM_DIST_CODES];


static uint16_t reverse_bits(uint16_t code, uint8_t bitlen)
{
    uint16_t reversed = 0;

    for (uint8_t i = 0; i < bitlen; i++)
    {
        reversed = (reversed << 1) | (code & 1);
        code >>= 1;
    }

    return reversed;
}

/*
 * Huffman Codes are prefix codes.
 * They DO NOT constitute as numbers, so they DO NOT follow RULE #1
 *
 * RULE #1: Whenever a numerical value is pushed into the bit stream 
 * (regardless of how many bits it occupies) 
 * that will always be pushed from LSB to MSB into the bit stream. 
 * This applies to all numerical values (like a binary sequence representing a number)
 * IT DOES NOT APPLY TO ARBITRARY BIT SEQUENCES (like Huffman / Prefix codes)
 *
 * So, it makes sense to pre-reverse the codes during intialization.
 * This allows us to push them using our bitwriter and therefore not need any extra functions.
 *
 * The HEX codes used to instantiate the codes can be found in Section 3.2.6 of RFC 1951
*/

void init_fixed_huffman_tables(void)
{
    for (uint16_t i = 0; i <= 143; i++)
    {
        ll_codes[i].bitlen = 8;
        ll_codes[i].code = reverse_bits(0x30 + i, 8);
    }

    for (uint16_t i = 144; i <= 255; i++)
    {
        ll_codes[i].bitlen = 9;
        ll_codes[i].code = reverse_bits(0x190 + (i - 144), 9);
    }

    for (uint16_t i = 256; i <= 279; i++)
    {
        ll_codes[i].bitlen = 7;
        ll_codes[i].code = reverse_bits(i - 256, 7);
    }

    for (uint16_t i = 280; i <= 287; i++)
    {
        ll_codes[i].bitlen = 8;
        ll_codes[i].code = reverse_bits(0xC0 + (i - 280), 8);
    }

    for (uint16_t i = 0; i < 32; i++)
    {
        dist_codes[i].bitlen = 5;
        dist_codes[i].code = reverse_bits(i, 5);
    }
}

void length_literal_to_code(uint16_t length, uint16_t *code,
        uint8_t *offset_bits, uint16_t *extra_value)
{
    if (length < 3) length = 3;
    if (length > 258) length = 258;

    for (int i = 0; i < 29; i++)
    {
        uint16_t range_start = ll_range_start[i];
        uint8_t offset = ll_offset_bits[i];
        uint16_t range_end;

        if (i < 28)
        {
            range_end = ll_range_start[i + 1];
        }

        else
        {
            range_end = 259;
        }

        if (length >= range_start && length < range_end)
        {
            *code = 257 + i;
            *offset_bits = offset;
            *extra_value = length - range_start;
            return;
        }
    }

    *code = 285;
    *offset_bits = 0;
    *extra_value = 0;
}

void dist_literal_to_code(uint16_t dist, uint16_t *code,
        uint8_t *offset_bits, uint16_t *extra_value)
{
    if (dist == 0) dist = 1;
    if (dist > 32768) dist = 32768;

    for (int i = 0; i < 30; i++)
    {
        uint16_t range_start = dist_range_start[i];
        uint8_t offset = dist_offset_bits[i];
        uint16_t range_end;

        if (i < 29)
        {
            range_end = dist_range_start[i + 1];
        }

        else
        {
            range_end = 32769;
        }

        if (dist >= range_start && dist < range_end)
        {
            *code = i;
            *offset_bits = offset;
            *extra_value = dist - range_start;
            return;
        }
    }

    *code = 29;
    *offset_bits = 0;
    *extra_value = 0;
}

HuffmanFixedCode get_fixed_literal_code(uint16_t symbol)
{
    return ll_codes[symbol];
}

HuffmanFixedCode get_fixed_dist_code(uint16_t dist)
{
    return dist_codes[dist];
}

void build_canonical_codes(const uint8_t *lengths, uint16_t *codes,
        int nsym)
{
    uint16_t bl_count[16] = {0};

    for (int i = 0; i < nsym; i++)
    {
        if (lengths[i] > 0)
        {
            bl_count[lengths[i]]++;
        }
    }

    uint16_t next_code[16];
    uint16_t code = 0;

    bl_count[0] = 0;

    for (int bits = 1; bits <= 15; bits++)
    {
        code = (code + bl_count[bits - 1]) << 1;
        next_code[bits] = code;
    }

    for (int i = 0; i < nsym; i++)
    {
        if (lengths[i] != 0)
        {
            codes[i] = reverse_bits(next_code[lengths[i]], lengths[i]);
            next_code[lengths[i]]++;
        }

        else
        {
            codes[i] = 0;
        }
    }
}
