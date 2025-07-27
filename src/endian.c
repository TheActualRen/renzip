#include "../include/endian.h"

void write_u16_le(uint8_t* buf, uint16_t val) {
	buf[0] = val & 0xFF;
	buf[1] = (val >> 8) & 0xFF;
}

void write_u32_le(uint8_t* buf, uint32_t val) {
	buf[0] = val & 0xFF;
	buf[1] = (val >> 8) & 0xFF;
	buf[2] = (val >> 16) & 0xFF;
	buf[3] = (val >> 24) & 0xFF;
}

int fwrite_u16_le(FILE* f, uint16_t val) {
	uint8_t buf[2];
	write_u16_le(buf, val);
	
	if (fwrite(buf, 1, 2, f) == 2) {
		return 0;
	}

	return -1;
}

int fwrite_u32_le(FILE* f, uint32_t val) {
	uint8_t buf[4];
	write_u32_le(buf, val);

	if (fwrite(buf, 1, 4, f) == 4) {
		return 0;
	}

	return -1;
}

uint16_t read_u16_le(const uint8_t* buf) {
	return (uint16_t)buf[0] | ((uint16_t)buf[1] >> 8);
}

uint32_t read_u32_le(const uint8_t* buf) {
	return (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) | 
		   ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
}
