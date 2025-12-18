#include "header.h"

#include <stdint.h>

GZIP_HEADER_STATUS write_gzip_header(FILE *output_file)
{
    uint8_t header_bytes[N_HEADER_BYTES] = 
    {
        0x1F,
        0x8B,
        0x08,
        0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00,
        0x03
    };

    size_t bytes_written = fwrite(header_bytes, sizeof(uint8_t), N_HEADER_BYTES, output_file);

    if (bytes_written != N_HEADER_BYTES)

    {
        return GZIP_HEADER_WRITE_FAILURE;
    }

    return GZIP_HEADER_SUCCESS;
}
