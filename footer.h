#ifndef FOOTER_H
#define FOOTER_H

#include <stdio.h>
#include <stdint.h>

#define GZIP_FOOTER_SUCCESS 0
#define GZIP_FOOTER_FAILURE 1

uint32_t update_crc(uint32_t crc, uint8_t *buf, int len);
int write_gzip_footer(FILE *output_file, uint32_t crc, uint32_t isize);

#endif
