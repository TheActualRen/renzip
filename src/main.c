#include "../include/blocktype0.h"

#include <stdio.h>
#include <stdlib.h>

int main() {
	FILE* in = fopen("../test/input.txt", "r");

	if (!in) {
		perror("Error Opening INPUT file");
		exit(1);
	}

	FILE* out = fopen("../test/output.gz", "wb");

	if (!out) {
		perror("Error Opening OUTPUT file");
		exit(1);
	}

	if (blocktype0_encoding(in, out) != 0) {
		fprintf(stderr, "Error: failed to complete BLOCK TYPE 0 encoding\n");
		fclose(in);
		fclose(out);
		exit(1);
	}

	printf("Succesful BLOCK TYPE 0 encoding\n");

	fclose(in);
	fclose(out);
}
