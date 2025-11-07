#include "bitwriter.h"

#include <stdio.h>

int bitwriter_init(BitWriter *bw, uint8_t *output_buf, size_t buf_size)
{
    if (!bw || !output_buf || buf_size == 0)
    {
        fprintf(stderr, "Failed BitWriter initialization\n");
        return BITWRITER_FAILED_INIT;
    }

    bw->buf = output_buf;
    bw->buf_size = buf_size;
    bw->byte_pos = 0;
    bw->bit_buf = 0;
    bw->bit_count = 0;

    return BITWRITER_SUCCESS;
}

int bitwriter_push_bits(BitWriter *bw, uint32_t bits, uint8_t num_bits)
{
    if (!bw || num_bits > 32) return BITWRITER_FAILED_PUSH_BIT;

    for (uint8_t i = 0; i < num_bits; i++)
    {
        uint8_t bit = (bits >> i) & 1;
        bw->bit_buf |= bit << bw->bit_count;
        bw->bit_count++;

        if (bw->bit_count == 8)
        {
            if (bw->byte_pos >= bw->buf_size)
            {
                fprintf(stderr, "Failed to write bits\n");
                return BITWRITER_FAILED_PUSH_BIT;
            }

            bw->buf[bw->byte_pos] = bw->bit_buf;
            bw->byte_pos++;

            bw->bit_buf = 0;
            bw->bit_count = 0;
        }
    }

    return BITWRITER_SUCCESS;
}

int bitwriter_push_bytes(BitWriter *bw, const uint8_t *data, size_t len)
{
    if (!bw || !data)
    {
        fprintf(stderr, "Invalid bitwriter or data pointer provided\n");
        return BITWRITER_FAILED_PUSH_BYTE;
    }

    if (bw->bit_count > 0)
    {
        if (bitwriter_flush(bw, NULL) != BITWRITER_SUCCESS)
        {
            return BITWRITER_FAILED_PUSH_BYTE;
        }
    }

    for (size_t i = 0; i < len; i++)
    {
        if (bitwriter_push_bits(bw, data[i], 8) != BITWRITER_SUCCESS)
        {
            fprintf(stderr, "Failed pushing byte %zu in bitwriter\n", i);
            return BITWRITER_FAILED_WRITE;
        }
    }

    return BITWRITER_SUCCESS;
}

int bitwriter_write_bits(BitWriter *bw, FILE *output_file)
{
    if (!bw || !output_file)
    {
        fprintf(stderr,
                "Invalid bitwriter or file pointer. Cannot write bits\n");
        return BITWRITER_FAILED_WRITE;
    }

    size_t bytes_written =
        fwrite(bw->buf, sizeof(uint8_t), bw->byte_pos, output_file);

    if (bytes_written != bw->byte_pos)
    {
        fprintf(stderr, "Failed to write bits to file\n");
        return BITWRITER_FAILED_WRITE;
    }

    bw->byte_pos = 0;

    return BITWRITER_SUCCESS;
}

int bitwriter_flush(BitWriter *bw, size_t *bytes_written)
{
    if (bw->bit_count > 0)
    {
        if (bw->byte_pos >= bw->buf_size)
        {
            fprintf(stderr, "Failed to flush BitWriter\n");
            return BITWRITER_FAILED_FLUSH;
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
