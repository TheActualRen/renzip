#ifndef BITWRITER_H
#define BITWRITER_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint8_t* buf;
	size_t buf_size;
	size_t byte_pos;

	uint8_t bit_buf;
	uint8_t bit_count;

	bool error;
} BitWriter;


void bitwriter_init(BitWriter* bw, uint8_t* output_buf,
					size_t buf_size); 

void bitwriter_write_bits(BitWriter* bw, uint32_t bits, 
						  uint8_t num_bits);

void bitwriter_align_byte(BitWriter* bw);
size_t bitwriter_flush(BitWriter* bw);

int bitwriter_has_error(const BitWriter* bw);

#endif
