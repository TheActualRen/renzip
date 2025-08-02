#include "../include/blocktype1.h"
#include "../include/header.h"
#include "../include/footer.h"
#include "../include/bitwriter.h"
#include "../include/huffman_fixed.h"
#include "../include/lz77.h"

#include <stdlib.h>
#include <sys/stat.h>

#define IS_LAST_BLOCK 1
#define IS_LAST_BLOCK_BIT_COUNT 1

#define BTYPE_BIT_COUNT 2
#define BTYPE_COMPRESSED 1

static const uint16_t length_base[29] = {
	3, 4, 5, 6, 7, 8, 9, 10,
    11, 13, 15, 17, 19, 23, 27, 31,
    35, 43, 51, 59, 67, 83, 99, 115,
    131, 163, 195, 227, 258
};

static const uint8_t length_extra_bits[29] = {
	0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 2, 2, 2, 2,
    3, 3, 3, 3, 4, 4, 4, 4,
    5, 5, 5, 5, 0
};

static void len_to_code(uint16_t length, uint16_t* code, uint8_t* extra_bits,
                    uint16_t* extra_value) {

	if (length < 3) length = 3;
	if (length > 258) length = 258;
	
	for (int i = 0; i < 29; i++) {
		uint16_t base = length_base[i];
		uint8_t bits = length_extra_bits[i];
		uint16_t next_base = (i < 28) ? length_base[i + 1] : 259;

		if (length >= base && length < next_base) {
			*code = 257 + i;
			*extra_bits = bits;
			*extra_value = length - base;
			return;
		}
	}
	*code = 285;
	*extra_bits = 0;
	*extra_value = 0;
}

static const uint16_t dist_base[30] = {
    1, 2, 3, 4, 5, 7, 9, 13,
    17, 25, 33, 49, 65, 97, 129, 193,
    257, 385, 513, 769, 1025, 1537, 2049, 3073,
    4097, 6145, 8193, 12289, 16385, 24577
};

static const uint8_t dist_extra_bits[30] = {
    0, 0, 0, 0, 1, 1, 2, 2,
    3, 3, 4, 4, 5, 5, 6, 6,
    7, 7, 8, 8, 9, 9, 10, 10,
    11, 11, 12, 12, 13, 13
};

static void dist_to_code(uint16_t distance, uint16_t* code, uint8_t* extra_bits,
                      uint16_t* extra_value) {

	if (distance == 0) { 
		distance = 1;
	}

	if (distance > 32768) {
		distance = 32768;
	}

	for (int i = 0; i < 30; i++) {
		uint16_t base = dist_base[i];
		uint8_t bits = dist_extra_bits[i];
		uint16_t next_base = (i < 29) ? dist_base[i + 1] : 32769;

		if (distance >= base && distance < next_base) {
			*code = i;
			*extra_bits = bits;
			*extra_value = distance - base;
			return;
		}
	}

	*code = 29;
	*extra_bits = 0;
	*extra_value = 0;
}

int blocktype1_encoding(FILE* in, FILE* out) {
	if (write_gzip_header(out) != 0) {
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
    uint8_t* input_buf = malloc(input_size);

	if (!input_buf) {
		perror("Memory Alloc for input buffer failed");
	}

	size_t bytes_read = fread(input_buf, 1, input_size, in);

	if (bytes_read != input_size) {
		fprintf(stderr, "Error: fread %zu bytes, expected %zu bytes\n", bytes_read, input_size);
		free(input_buf);
		return -1;
	}

	uint32_t crc = crc32(0, input_buf, input_size);

	size_t compressed_size = input_size * 2 + 100;
    uint8_t* compressed = malloc(compressed_size);

	if (!compressed) {
		perror("Memory Alloc for compressed failed");
		free(input_buf);
		return -1;
	}

    LZ77TokenList tokens = {0};
    lz77_compress(input_buf, input_size, &tokens);

    BitWriter bw;
    bitwriter_init(&bw, compressed, compressed_size);

	uint8_t bfinal = IS_LAST_BLOCK & 1;
	uint8_t btype = BTYPE_COMPRESSED & 0x03; 
	
	uint8_t header = (bfinal) | (btype << IS_LAST_BLOCK_BIT_COUNT);

    bitwriter_write_bits(&bw, header, IS_LAST_BLOCK_BIT_COUNT + BTYPE_BIT_COUNT);

	if (bitwriter_has_error(&bw)) {
		fprintf(stderr, "Error: Bitwriter buffer overflow when writing the header\n");
		free(input_buf);
		free(compressed);
		return -1;
	}

    init_fixed_huffman_tables();

    for (size_t i = 0; i < tokens.count; i++) {
        if (tokens.tokens[i].type == LZ77_LITERAL) {
			HuffmanFixedCode hcode = get_fixed_literal_code(tokens.tokens[i].literal);
			bitwriter_write_bits(&bw, hcode.code, hcode.bitlen);

			if (bitwriter_has_error(&bw)) {
				fprintf(stderr, "Error: Bitwriter buffer overflow when writing Huffman Codes\n");
				free(input_buf);
				free(compressed);
				return -1;
			}
			
        } else if (tokens.tokens[i].type == LZ77_MATCH) {
            uint16_t len = tokens.tokens[i].match.len;
            uint16_t dist = tokens.tokens[i].match.dist;

            uint16_t len_code;
            uint8_t len_extra_bits;
            uint16_t len_extra_value;

            len_to_code(len, &len_code, &len_extra_bits,
                         &len_extra_value);
          
            HuffmanFixedCode len_huff = get_fixed_literal_code(len_code);
            bitwriter_write_bits(&bw, len_huff.code, len_huff.bitlen);

			if (bitwriter_has_error(&bw)) {
				fprintf(stderr, "Error: Bitwritter buffer overflow when writing LEN HUFFMAN CODES\n");
				free(input_buf);
				free(compressed);
				return -1;
			}

            if (len_extra_bits > 0) {
                bitwriter_write_bits(&bw, len_extra_value, len_extra_bits);
				
				if (bitwriter_has_error(&bw)) {
					fprintf(stderr, "Error: Bitwriter buffer overflow when writing EXTRA LEN BITS\n");
					free(input_buf);
					free(compressed);
					return -1;
				}
            }

            uint16_t dist_code;
            uint8_t dist_extra_bits;
            uint16_t dist_extra_value;

            dist_to_code(dist, &dist_code, &dist_extra_bits,
                         &dist_extra_value);

            HuffmanFixedCode dist_huff = get_fixed_dist_code(dist_code);

            bitwriter_write_bits(&bw, dist_huff.code, dist_huff.bitlen);

			if (bitwriter_has_error(&bw)) {
				fprintf(stderr, "Error: Bitwriter buffer overflow when writing DIST HUFFMAN CODES\n");
				free(input_buf);
				free(compressed);
				return -1;
			}

            if (dist_extra_bits > 0) {
                bitwriter_write_bits(&bw, dist_extra_value, dist_extra_bits);

				if (bitwriter_has_error(&bw)) {
					fprintf(stderr, "Error: Bitwriter buffer overflow when writing EXTRA DIST BITS\n");
					free(input_buf);
					free(compressed);
					return -1;
				}
            }
        }
    }

    HuffmanFixedCode eob = get_fixed_literal_code(256);

    bitwriter_write_bits(&bw, eob.code, eob.bitlen);

	if (bitwriter_has_error(&bw)) {
		fprintf(stderr, "Error: Bitwriter buffer overflow when writing EOB CODE");
		free(input_buf);
		free(compressed);
		return -1;
	}

    bitwriter_align_byte(&bw);

	if (bitwriter_has_error(&bw)) {
		fprintf(stderr, "Error: Bitwriter buffer overflow when aligning byte\n");
		free(input_buf);
		free(compressed);
		return -1;
	}

    size_t final_size = bitwriter_flush(&bw);
    fwrite(compressed, 1, final_size, out);
    lz77_free_tokens(&tokens);

	if (write_gzip_footer(out, crc, input_size) != 0) {
		fprintf(stderr, "Error writing gzip footer\n");
		free(input_buf);
		free(compressed);
		return -1;
	}

	free(input_buf);
	free(compressed);

	return 0;
}
