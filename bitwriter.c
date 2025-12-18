#include "bitwriter.h"

BITWRITER_STATUS bitwriter_init(BitWriter *bw, uint8_t *output_buf, size_t buf_size)
{
    if (!bw || !output_buf || buf_size == 0)
    {
        return BITWRITER_INIT_FAILURE;
    }

    bw->buf = output_buf;
    bw->buf_size = buf_size;
    bw->byte_pos = 0;
    bw->bit_buf = 0;
    bw->bit_count = 0;

    return BITWRITER_SUCCESS;
}

BITWRITER_STATUS bitwriter_push_bits(BitWriter *bw, uint32_t bits, uint8_t num_bits)
{
    if (!bw || num_bits > 32)
    {
        return BITWRITER_PUSH_BIT_FAILURE;
    }

    for (uint8_t i = 0; i < num_bits; i++)
    {
        uint8_t bit = (bits >> i) & 1;
        bw->bit_buf |= bit << bw->bit_count;
        bw->bit_count++;

        if (bw->bit_count == 8)
        {
            if (bw->byte_pos >= bw->buf_size)
            {
                return BITWRITER_PUSH_BIT_FAILURE;
            }

            bw->buf[bw->byte_pos] = bw->bit_buf;
            bw->byte_pos++;

            bw->bit_buf = 0;
            bw->bit_count = 0;
        }
    }

    return BITWRITER_SUCCESS;
}

BITWRITER_STATUS bitwriter_push_bytes(BitWriter *bw, const uint8_t *data, size_t len)
{
    if (!bw || !data)
    {
        return BITWRITER_PUSH_BYTE_FAILURE;
    }

    if (bw->bit_count > 0)
    {
        if (bitwriter_flush(bw, NULL) != BITWRITER_SUCCESS)
        {
            return BITWRITER_PUSH_BYTE_FAILURE;
        }
    }

    for (size_t i = 0; i < len; i++)
    {
        if (bitwriter_push_bits(bw, data[i], 8) != BITWRITER_SUCCESS)
        {
            return BITWRITER_PUSH_BYTE_FAILURE;
        }
    }

    return BITWRITER_SUCCESS;
}

BITWRITER_STATUS bitwriter_write_bits(BitWriter *bw, FILE *output_file)
{
    if (!bw || !output_file)
    {
        return BITWRITER_WRITE_FAILURE;
    }

    size_t bytes_written = fwrite(bw->buf, sizeof(uint8_t), bw->byte_pos, output_file);

    if (bytes_written != bw->byte_pos)
    {
        return BITWRITER_WRITE_FAILURE;
    }

    bw->byte_pos = 0;

    return BITWRITER_SUCCESS;
}

BITWRITER_STATUS bitwriter_flush(BitWriter *bw, size_t *bytes_written)
{
    if (bw->bit_count > 0)
    {
        if (bw->byte_pos >= bw->buf_size)
        {
            return BITWRITER_FLUSH_FAILURE;
        }

        bw->buf[bw->byte_pos] = bw->bit_buf;
        bw->byte_pos++;

        bw->bit_buf = 0;
        bw->bit_count = 0;
    }

    if (bytes_written)
    {
        *bytes_written = bw->byte_pos;
    }

    return BITWRITER_SUCCESS;
}
