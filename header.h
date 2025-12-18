#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>

#define N_HEADER_BYTES 10

typedef enum
{
    GZIP_HEADER_SUCCESS = 0,
    GZIP_HEADER_WRITE_FAILURE

} GZIP_HEADER_STATUS;

GZIP_HEADER_STATUS write_gzip_header(FILE *output_file);

#endif
