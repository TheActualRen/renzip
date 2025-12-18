#include "blocktype0.h"

#include "header.h"
#include "bitwriter.h"
#include "footer.h"

#include <stdlib.h>
#include <stdint.h>

B0_STATUS blocktype0_encoding(FILE *input_file, FILE *output_file)
{
    B0_STATUS status = B0_SUCCESS;

    GZIP_HEADER_STATUS header_status = write_gzip_header(output_file);

    if (header_status != GZIP_HEADER_SUCCESS)
    {
        return B0_HEADER_FAILURE;
    }

    fseek(input_file, 0, SEEK_END);
    size_t input_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    if (input_size > MAX_SIZE)
    {
        return B0_INVALID_INPUT;
    }

    uint8_t *input_buf = malloc(input_size);

    if (!input_buf)
    {
        status = B0_MALLOC_FAILURE;
        goto cleanup;
    }

    size_t bytes_read = fread(input_buf, 1, input_size, input_file);

    if (!bytes_read || bytes_read != input_size)
    {
        status = B0_READ_FAILURE;
        goto cleanup;
    }

    uint8_t output_buf[MAX_SIZE + 32]; // "+ 32" is padding for the DEFLATE header

    BitWriter bw;
    BITWRITER_STATUS bw_status = bitwriter_init(&bw, output_buf, sizeof(output_buf));

    if (bw_status != BITWRITER_SUCCESS)
    {
        status = B0_WRITING_FAILURE;
        goto cleanup;
    }

    bw_status = bitwriter_push_bits(&bw, IS_LAST_BIT, IS_LAST_NUM_BITS); 
    if (bw_status != BITWRITER_SUCCESS)
    {
        status = B0_WRITING_FAILURE;
        goto cleanup;
    }

    bw_status = bitwriter_push_bits(&bw, BTYPE0, BTYPE0_NUM_BITS);
    if (bw_status != BITWRITER_SUCCESS)
    {
        status = B0_WRITING_FAILURE;
        goto cleanup;
    }

    bw_status = bitwriter_push_bits(&bw, HEADER_PADDING, HEADER_PADDING_NUM_BITS);
    if (bw_status != BITWRITER_SUCCESS)
    {
        status = B0_WRITING_FAILURE;
        goto cleanup;
    }

    uint16_t len = (uint16_t)input_size;
    uint16_t nlen = ~len;

    bw_status = bitwriter_push_bits(&bw, len, 16);
    if (bw_status != BITWRITER_SUCCESS)
    {
        status = B0_WRITING_FAILURE;
        goto cleanup;
    }

    bw_status = bitwriter_push_bits(&bw, nlen, 16);
    if (bw_status != BITWRITER_SUCCESS)
    {
        status = B0_WRITING_FAILURE;
        goto cleanup;
    }

    bw_status = bitwriter_push_bytes(&bw, input_buf, len);
    if (bw_status != BITWRITER_SUCCESS)
    {
        status = B0_WRITING_FAILURE;
    }

    size_t bytes_written;
    bw_status = bitwriter_flush(&bw, &bytes_written); 

    if (bw_status != BITWRITER_SUCCESS)
    {
        status = B0_WRITING_FAILURE;
        goto cleanup;
    }

    bw_status = bitwriter_write_bits(&bw, output_file); 
    if (bw_status != BITWRITER_SUCCESS)
    {
        status = B0_WRITING_FAILURE;
        goto cleanup;
    }

    uint32_t crc = update_crc(0, input_buf, len);

    if (write_gzip_footer(output_file, crc, len) != GZIP_FOOTER_SUCCESS)
    {
        status = B0_FOOTER_FAILURE;
        goto cleanup;
    }

cleanup:
    free(input_buf);

    return status;
}
