#ifndef FILE_HANDLING_H
#define FILE_HANDLING_H

#include <stdio.h>

typedef enum
{
    FILE_HANDLING_SUCCESS = 0,
    FILE_HANDLING_FAILURE

} FILE_HANDLING_STATUS;


FILE_HANDLING_STATUS readfile_binary(FILE **file, char *filename);
FILE_HANDLING_STATUS writefile_binary(FILE **file, char *filename);

#endif
