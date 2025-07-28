#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "../include/blocktype0.h"
#include "../include/header.h"
#include "../include/footer.h"
#include "../include/bitwriter.h"
#include "../include/endian.h"

#define MAX_LEN 65535
#define DEFLATE_HEADER_BUF_SIZE 16

#define IS_LAST_BLOCK 1
#define IS_LAST_BLOCK_BIT_COUNT 1

#define BTYPE_BIT_COUNT 2
#define BTYPE_UNCOMPRESSED 0

int blocktype0_encoding(FILE* in, FILE* out) {

	if(write_gzip_header(out) != 0) {
		fprintf(stderr, "Error writing gzip header\n");
		return -1;
	}

	int fd = fileno(in);
	struct stat st;

	if (fstat(fd, &st) != 0) {
		perror("fstat failed");
		return -1;
	}

	size_t input_size = st.st_size;

	if (input_size > MAX_LEN) {
		fprintf(stderr, "Error: Block type 0 cannot exceed 64KB\n");
		return -1;	
	}

	uint8_t* input_data = malloc(input_size);

	if (!input_data) {
		fprintf(stderr, "Error: malloc failed to allocate %zu bytes\n", input_size);
		return -1;	
	}

	size_t bytes_read = fread(input_data, 1, input_size, in);

	if (!bytes_read || bytes_read != input_size) {
		fprintf(stderr, "Error: fread read %zu bytes, expected %zu\n", bytes_read, input_size);
		free(input_data);
		return -1;
	}	
	
	uint8_t deflate_header_buf[DEFLATE_HEADER_BUF_SIZE] = {0};
	BitWriter bw;
	bitwriter_init(&bw, deflate_header_buf, DEFLATE_HEADER_BUF_SIZE);
	bitwriter_write_bits(&bw, IS_LAST_BLOCK, IS_LAST_BLOCK_BIT_COUNT); 
	bitwriter_write_bits(&bw, BTYPE_UNCOMPRESSED, BTYPE_BIT_COUNT); 
	bitwriter_align_byte(&bw);

	size_t header_len = bitwriter_flush(&bw);

	if (fwrite(deflate_header_buf, 1, header_len, out) != header_len) {
		fprintf(stderr, "Error writing deflate header\n");
		free(input_data);
		return -1;
	}

	uint16_t len = (uint16_t)input_size;
	uint16_t nlen = ~len;

	if (fwrite_u16_le(out, len) != 0) {
		fprintf(stderr, "Error: failed to write len to file\n");
		free(input_data);
		return -1;
	}

	if (fwrite_u16_le(out, nlen) != 0) {
		fprintf(stderr, "Error: failed to write nlen to file\n");
		free(input_data);
		return -1;
	}

	if (fwrite(input_data, 1, len, out) != len) {
		fprintf(stderr, "Error: failed to write input data to file\n");
		free(input_data);
		return -1;
	}

	uint32_t crc = crc32(0, input_data, len);

	if (write_gzip_footer(out, crc, len) != 0) {
		fprintf(stderr, "Error writing gzip footer\n");
		return -1;
	}

	free(input_data);

	return 0;
}
