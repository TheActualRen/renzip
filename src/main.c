#include "../include/blocktype0.h"

#include <stdio.h>
#include <stdlib.h>

int main() {
	FILE* in = fopen("../test/input.txt", "r");

	if (in == NULL) {
		perror("Error Opening INPUT file");
		exit(1);
	}

	FILE* out = fopen("../test/output.gz", "wb");

	if (out == NULL) {
		perror("Error Opening OUTPUT file");
		exit(1);
	}

	blocktype0_encoding(in, out);

	fclose(in);
	fclose(out);
}
