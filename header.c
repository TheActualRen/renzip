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

GZIP_HEADER_STATUS read_gzip_header(FILE *input_file)
{
    uint8_t header[10];

    size_t n = fread(header, 1, 10, input_file);

    if (n != 10) 
    {
        return GZIP_HEADER_READ_FAILURE;
    }

    if (header[0] != 0x1f || header[1] != 0x8B) 
    {
        return GZIP_HEADER_READ_FAILURE;
    }

    if (header[2] != 0x08) 
    {
        return GZIP_HEADER_READ_FAILURE;
    }

    if (header[3] != 0x00) 
    {
        return GZIP_HEADER_READ_FAILURE;
    }

    if (header[9] != 0x03) 
    {
        return GZIP_HEADER_READ_FAILURE;
    }

    return GZIP_HEADER_SUCCESS;
}

GZIP_HEADER_STATUS read_gzip_header_true(FILE *input_file)
{
    uint8_t magic_byte1, magic_byte2, cm, flg;
    uint32_t mtime;
    uint8_t xfl, os;

    GZIP_HEADER_STATUS header_status = 
        fread(&magic_byte1, 1, 1, input_file);

    if (header_status != 1)
    {
        return GZIP_HEADER_READ_FAILURE;
    }

    header_status = fread(&magic_byte2, 1, 1, input_file);

    if (header_status != 1)
    {
        return GZIP_HEADER_READ_FAILURE;
    }

    if (magic_byte1 != 0x1F || magic_byte2 != 0x8B)
    {
        return GZIP_INVALID_HEADER; 
    }

    header_status = fread(&cm, 1, 1, input_file);

    if (header_status != 1)
    {
        return GZIP_HEADER_READ_FAILURE;
    }

    if (cm != 0x08)
    {
        return GZIP_INVALID_HEADER;
    }

    header_status = fread(&flg, 1, 1, input_file);

    if (header_status != 1)
    {
        return GZIP_HEADER_READ_FAILURE;
    }

    header_status = fread(&mtime, 4, 1, input_file);
    
    if (header_status != 1)
    {
        return GZIP_HEADER_READ_FAILURE;
    }

    header_status = fread(&xfl, 1, 1, input_file);

    if (header_status != 1)
    {
        return GZIP_HEADER_READ_FAILURE;
    }

    header_status = fread(&os, 1, 1, input_file);

    if (header_status != 1)
    {
        return GZIP_HEADER_READ_FAILURE;
    }

    /* Optional extra field */
    if (flg & 0x04)
    {
        uint16_t xlen;

        if (fread(&xlen, 2, 1, input_file) != 1) 
        {
            return GZIP_HEADER_READ_FAILURE;
        }

        if (fseek(input_file, xlen, SEEK_CUR) != 0) 
        {
            return GZIP_HEADER_READ_FAILURE;
        }

    }

    /* Optional filename */
    if (flg & 0x08)
    {
        int c;

        do 
        {
            c = fgetc(input_file);

            if (c == EOF) 
            {
                return GZIP_HEADER_READ_FAILURE;
            }

        } while (c != 0);
    }

    /* Optional comment */
    if (flg & 0x10)
    {
        int c;
        do 
        {
            c = fgetc(input_file);

            if (c == EOF) 
            {
                return GZIP_HEADER_READ_FAILURE;
            }

        } while (c != 0);
    }

    /* Optional header CRC */
    if (flg & 0x02)
    {
        if (fseek(input_file, 2, SEEK_CUR) != 0) 
        {
            return GZIP_HEADER_READ_FAILURE;
        }

    }

    return GZIP_HEADER_SUCCESS;
}
