#include "bitreader.h"

#include <stdio.h>

BITREADER_STATUS bitreader_init(BitReader *br, uint8_t *input_buf, 
        size_t buf_size)
{
    if (!br || !input_buf || buf_size == 0) 
    {
        return BITREADER_FAILED_INIT;
    }

    br->buf = input_buf;
    br->buf_size = buf_size;
    br->byte_pos = 0;
    br->bit_buf = 0;
    br->bit_count = 0;

    return BITREADER_SUCCESS;
}

BITREADER_STATUS bitreader_read_bits(BitReader *br, uint32_t *bits,
        uint8_t num_bits)
{
    if (!br || num_bits > 32) 
    {
        return BITREADER_FAILED_READ_BIT;
    }

    uint32_t result = 0;

    for (uint8_t i = 0; i < num_bits; i++)
    {
        if (br->bit_count == 0)
        {
            if (br->byte_pos >= br->buf_size)
            {
                return BITREADER_OUT_OF_DATA;
            }

            br->bit_buf = br->buf[br->byte_pos++];
            br->bit_count = 8;
        }

        uint8_t bit = br->bit_buf & 1;
        br->bit_buf >>= 1;
        br->bit_count--;

        result |= (bit << i);
    }

    *bits = result;
    return BITREADER_SUCCESS;
}

BITREADER_STATUS bitreader_read_bytes(BitReader *br, uint8_t *out, size_t len)
{
    if (!br || !out) 
    {
        return BITREADER_FAILED_READ_BYTE;
    }

    if (br->bit_count > 0) 
    {
        br->bit_buf = 0;
        br->bit_count = 0;
    }

    if (br->byte_pos + len > br->buf_size) 
    {
        return BITREADER_OUT_OF_DATA;
    }

    for (size_t i = 0; i < len; i++) 
    {
        out[i] = br->buf[br->byte_pos++];
    }

    return BITREADER_SUCCESS;
}

BITREADER_STATUS bitreader_peek_bits(BitReader *br, uint32_t *out, uint8_t num_bits)
{
    if (!br || num_bits > 32) 
    {
        return BITREADER_FAILED_READ_BIT;
    }

    uint32_t saved_bit_buf = br->bit_buf;
    uint8_t  saved_bit_count = br->bit_count;
    size_t   saved_byte_pos = br->byte_pos;

    uint32_t tmp = 0;
    int res = bitreader_read_bits(br, &tmp, num_bits);

    // restore state
    br->bit_buf = saved_bit_buf;
    br->bit_count = saved_bit_count;
    br->byte_pos = saved_byte_pos;

    if (res != BITREADER_SUCCESS)
    {
        return res;
    }

    *out = tmp;
    return BITREADER_SUCCESS;
}

BITREADER_STATUS bitreader_drop_bits(BitReader *br, uint8_t num_bits)
{
    uint32_t throwaway;
    return bitreader_read_bits(br, &throwaway, num_bits);
}
