#include "header.h"

#include <stdint.h>

int write_gzip_header(FILE *output_file)
{   
    uint8_t header_bytes[10] = 
    {
        0x1f,
        0x8b,
        0x08,
        0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00,
        0x03
    };

    if (!fwrite(header_bytes, sizeof(uint8_t), 10, output_file))
    {
        fprintf(stderr, "Error writing the gzip header\n");
        return GZIP_HEADER_FAILURE;
    }

    return GZIP_HEADER_SUCCESS;
}
