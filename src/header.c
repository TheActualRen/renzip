#include "../include/header.h"

#define DEFLATE_COMPRESSION 0x08
#define OS_UNIX 0x03

int write_gzip_header(FILE* out) {
	uint8_t header[10] = {
		0x1f, // MAGIC Byte 1
		0x8b, // MAGIC BYTE 2
		DEFLATE_COMPRESSION,
		0x00, // Flags
		0x00,0x00, 0x00, 0x00, // MTIME 
		0x00, // Xtra Flags
		OS_UNIX	
	};

	size_t written = fwrite(header, sizeof(header), 1, out);

	if (written != 1){
		fprintf(stderr, "Error: Failed to write gzip header");
		return -1;
	} 

	return 0;
}

