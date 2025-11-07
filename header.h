#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>

#define GZIP_HEADER_SUCCESS 0
#define GZIP_HEADER_FAILURE 1

int write_gzip_header(FILE *output_file);

#endif
