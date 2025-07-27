#ifndef FOOTER_H
#define FOOTER_H

#include <stdio.h>
#include <stdint.h>

uint32_t crc32(uint32_t crc, unsigned char* buf, int len);
void write_gzip_footer(FILE* out, uint32_t crc, uint32_t isize);

#endif
