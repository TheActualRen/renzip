#include "../include/header.h"

int write_gzip_header(FILE* out) {
	uint8_t header[10] = {
		0x1f,
		0x8b,
		0x08,
		0x00,
		0x00,0x00, 0x00, 0x00,
		0x00,
		0x03
	};

	size_t written = fwrite(header, sizeof(header), 1, out);

	if (written != 1){
		fprintf(stderr, "Error: Failed to write gzip header");
		return -1;
	} 

	return 0;
}

