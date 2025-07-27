#ifndef ENDIAN_H
#define ENDIAN_H

#include <stdint.h>
#include <stdio.h>

void write_u16_le(uint8_t* buf, uint16_t val);
void write_u32_le(uint8_t* buf, uint32_t val);

int fwrite_u16_le(FILE* f, uint16_t val);
int fwrite_u32_le(FILE* f, uint32_t val);

uint16_t read_u16_le(const uint8_t* buf);
uint32_t read_u32_le(const uint8_t* buf);

#endif
