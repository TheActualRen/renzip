#include "../include/blocktype0.h"
#include "../include/header.h"
#include "../include/footer.h"
#include "../include/bitwriter.h"

#include <stdlib.h>

#define MAX_LEN 65535

// how to figure out we are in the last byte
// BTYPE is 00 for block type 0
// LEN we can read the size of the input file
// ~LEN
// Bitstream
// RULE#1 with pushing values into the bitstream

void blocktype0_encoding(FILE* in, FILE* out) {
	write_gzip_header(out);

	fseek(in, 0, SEEK_END);
	size_t input_size = ftell(in);
	rewind(in);

	if (input_size > MAX_LEN) {
		fprintf(stderr, "Error: Block type 0 cannot exceed 64KB\n");
		exit(1);
	}

	uint8_t* input_data = malloc(input_size);
	fread(input_data, 1, input_size, in);
	
	uint8_t deflate_header_buf[16] = {0};
	BitWriter bw;
	bitwriter_init(&bw, deflate_header_buf, sizeof(deflate_header_buf));
	bitwriter_write_bits(&bw, 1, 1); // IS_LAST = 1
	bitwriter_write_bits(&bw, 0, 2); // BTYPE = 00
	bitwriter_align_byte(&bw);

	size_t header_len = bitwriter_flush(&bw);
	fwrite(deflate_header_buf, 1, header_len, out);

	uint16_t len = (uint16_t)input_size;
	uint16_t nlen = ~len;

	uint8_t len_bytes[4] = {
		len & 0xFF,
		(len >> 8),
		nlen & 0xFF,
		(nlen >> 8) & 0xFF
	};

	fwrite(len_bytes, 1, 4, out);
	fwrite(input_data, 1, len, out);

	uint32_t crc = crc32(0, input_data, len);
	write_gzip_footer(out, crc, len);

	free(input_data);
}
