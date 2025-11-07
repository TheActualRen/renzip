#ifndef FILE_HANDLING_H
#define FILE_HANDLING_H

#include <stdio.h>

#define FILE_HANDLING_SUCCESS 0
#define FILE_HANDLING_FAILURE 1

int readfile_binary(FILE **file, char *filename);
int writefile_binary(FILE **file, char *filename);

#endif
