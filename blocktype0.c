#include "blocktype0.h"
#include "bitwriter.h"
#include "footer.h"
#include "header.h"

#include <stdint.h>
#include <stdlib.h>

int blocktype0_encoding(FILE *input_file, FILE *output_file)
{
    if (write_gzip_header(output_file) != GZIP_HEADER_SUCCESS)
    {
        fprintf(stderr, "Error writing gzip header, Block Type 0\n");
        return BLOCKTYPE0_HEADER_FAILURE;
    }

    fseek(input_file, 0, SEEK_END);
    size_t input_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    if (input_size > MAX_SIZE)
    {
        fprintf(stderr, "Block Type 0 cannot exceed 64KB\n");
        return BLOCKTYPE0_INVALID_INPUT;
    }

    uint8_t *input_buf = malloc(input_size);

    if (!input_buf)
    {
        fprintf(stderr, "Memory allocation failed for input buffer\n");
        return BLOCKTYPE0_MALLOC_FAILURE;
    }

    size_t bytes_read = fread(input_buf, 1, input_size, input_file);

    if (!bytes_read || bytes_read != input_size)
    {
        fprintf(stderr, "Error: fread read %zu bytes, expected %zu bytes",
                bytes_read, input_size);
        free(input_buf);
        return BLOCKTYPE0_READ_FAILURE;
    }

    uint8_t output_buf[MAX_SIZE + 32];
    BitWriter bw;

    if (bitwriter_init(&bw, output_buf, sizeof(output_buf)) !=
        BITWRITER_SUCCESS)
    {
        fprintf(stderr, "Error initializing bitwriter\n");
        free(input_buf);
        return BLOCKTYPE0_WRITING_FAILURE;
    }

    if (bitwriter_push_bits(&bw, IS_LAST_BIT, IS_LAST_NUM_BITS) !=
        BITWRITER_SUCCESS)
    {
        fprintf(stderr, "Error pushing bits\n");
        free(input_buf);
        return BLOCKTYPE0_WRITING_FAILURE;
    }

    if (bitwriter_push_bits(&bw, BTYPE0, BTYPE0_NUM_BITS) != BITWRITER_SUCCESS)
    {
        fprintf(stderr, "Error pushing bits\n");
        free(input_buf);
        return BLOCKTYPE0_WRITING_FAILURE;
    }

    if (bitwriter_push_bits(&bw, HEADER_PADDING, HEADER_PADDING_NUM_BITS) !=
        BITWRITER_SUCCESS)
    {
        fprintf(stderr, "Error pushing bits\n");
        free(input_buf);
        return BLOCKTYPE0_WRITING_FAILURE;
    }

    uint16_t len = (uint16_t)input_size;
    uint16_t nlen = ~len;

    if (bitwriter_push_bits(&bw, len, 16) != BITWRITER_SUCCESS)
    {
        fprintf(stderr, "Error pushing bits\n");
        free(input_buf);
        return BLOCKTYPE0_WRITING_FAILURE;
    }

    if (bitwriter_push_bits(&bw, nlen, 16) != BITWRITER_SUCCESS)
    {
        fprintf(stderr, "Error pushing bits\n");
        free(input_buf);
        return BLOCKTYPE0_WRITING_FAILURE;
    }

    if (bitwriter_push_bytes(&bw, input_buf, len) != BITWRITER_SUCCESS)
    {
        fprintf(stderr, "Error pushing bits\n");
        free(input_buf);
        return BLOCKTYPE0_WRITING_FAILURE;
    }

    size_t bytes_written;

    if (bitwriter_flush(&bw, &bytes_written) != BITWRITER_SUCCESS)
    {
        fprintf(stderr, "Error flushing bits\n");
        free(input_buf);
        return BLOCKTYPE0_WRITING_FAILURE;
    }

    if (bitwriter_write_bits(&bw, output_file) != BITWRITER_SUCCESS)
    {
        fprintf(stderr, "Error writing bits to file\n");
        free(input_buf);
        return BLOCKTYPE0_WRITING_FAILURE;
    }

    uint32_t crc = update_crc(0, input_buf, len);

    if (write_gzip_footer(output_file, crc, len) != GZIP_FOOTER_SUCCESS)
    {
        fprintf(stderr, "Error writing gzip footer for Block Type 0\n");
        free(input_buf);
        return BLOCKTYPE0_FOOTER_FAILURE;
    }

    free(input_buf);

    return BLOCKTYPE0_SUCCESS;
}
