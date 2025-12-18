#ifndef BLOCKTYPE_0
#define BLOCKTYPE_0

#include <stdio.h>

#define MAX_SIZE 65535

#define IS_LAST_BIT 0x01
#define IS_LAST_NUM_BITS 1

#define BTYPE0 0x00
#define BTYPE0_NUM_BITS 2

#define HEADER_PADDING 0x00
#define HEADER_PADDING_NUM_BITS 5

typedef enum
{
    B0_SUCCESS = 0,
    B0_HEADER_FAILURE,
    B0_INVALID_INPUT,
    B0_MALLOC_FAILURE,
    B0_READ_FAILURE,
    B0_WRITING_FAILURE,
    B0_FOOTER_FAILURE,
    B0_INVALID_BLOCKTYPE

} B0_STATUS;

B0_STATUS blocktype0_encoding(FILE *input_file, FILE *output_file);
B0_STATUS blocktype0_decoding(FILE *input_file, FILE *output_file);

#endif
