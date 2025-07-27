#include "../include/header.h"

void write_gzip_header(FILE* out) {
	uint8_t header[10] = {
		0x1f,
		0x8b,
		0x08,
		0x00,
		0x00,0x00, 0x00, 0x00,
		0x00,
		0x03
	};

	fwrite(header, sizeof(header), 1, out);
}
