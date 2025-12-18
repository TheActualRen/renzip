#ifndef FOOTER_H
#define FOOTER_H

#include <stdio.h>
#include <stdint.h>

typedef enum
{
    GZIP_FOOTER_SUCCESS = 0,
    GZIP_CRC_WRITE_FAILURE,
    GZIP_ISIZE_WRITE_FAILURE

} GZIP_FOOTER_STATUS;

uint32_t update_crc(uint32_t crc, uint8_t *buf, size_t len);
GZIP_FOOTER_STATUS write_gzip_footer(FILE *output_file, uint32_t crc, uint32_t isize);

#endif

