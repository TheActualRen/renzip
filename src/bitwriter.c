#include "../include/bitwriter.h"

void bitwriter_init(BitWriter* bw, uint8_t* output_buf, 
					size_t buf_size) {

	bw->buf = output_buf;
	bw->buf_size = buf_size;
	bw->byte_pos = 0;
	bw->bit_buf = 0;
	bw->bit_count = 0;
}

void bitwriter_write_bits(BitWriter* bw, uint32_t bits,
						  uint8_t num_bits) {
	
	for (uint8_t i = 0; i < num_bits; i++) {
		uint8_t bit = (bits >> i) & 1; // extracts LSB-first
		bw->bit_buf |= bit << bw->bit_count;
		bw->bit_count++;

		if (bw->bit_count == 8) {
			if (bw->byte_pos < bw->buf_size) {
				bw->buf[bw->byte_pos++] = bw->bit_buf;
			}
			bw->bit_buf = 0;
			bw->bit_count = 0;
		}
	}
}

void bitwriter_align_byte(BitWriter* bw) {
	if (bw->bit_count > 0) {
		if (bw->byte_pos < bw->buf_size) {
			bw->buf[bw->byte_pos++] = bw->bit_buf;
		}
		bw->bit_buf = 0;
		bw->bit_count = 0;
	}
}

size_t bitwriter_flush(BitWriter* bw) {
	if (bw->bit_count > 0) {
		if (bw->byte_pos < bw->buf_size) {
			bw->buf[bw->byte_pos++] = bw->bit_buf;
		}
		bw->bit_buf = 0;
		bw->bit_count = 0;
	}

	return bw->byte_pos;
}
