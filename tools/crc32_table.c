#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

uint32_t crc_table[256];
bool crc_table_computed = false;
uint32_t polynomial = 0xEDB88320;

void write_table_to_file(FILE* in, uint32_t* values) {
	fprintf(in, "#include <stdint.h>\n");
	fprintf(in, "static uint32_t crc_table[256] = {");
	
	for (int i = 0; i < 256; i++) {
		if (i % 4 == 0) {
			fprintf(in, "\n    0x%.8X, ", values[i]);
		} else {
			fprintf(in, "0x%.8X, ", values[i]);
		}
	}

	fprintf(in, "\n};");
}

void make_crc_table(FILE* in) {
	uint32_t c;

	int n, k;

	for (n = 0; n < 256; n++) {
		c = (uint32_t) n;

		for (k = 0; k < 8; k++) {
			if (c & 1) {
				c = polynomial ^ (c >> 1);
			} else {
				c >>= 1;
			}
		}
		crc_table[n] = c;
		printf("0x%.8X\n", crc_table[n]);
	}
	crc_table_computed = true;

	uint32_t* ptrValues = crc_table;
	write_table_to_file(in, ptrValues);

}

int main() {
	FILE* in = fopen("table.c", "w");

	if (in == NULL) {
		perror("Error Opening file");
		return 1;
	}

	make_crc_table(in);
	fclose(in);
}
