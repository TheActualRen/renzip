#include "file_handling.h"

int readfile_binary(FILE **file, char *filename)
{
    FILE *f = fopen(filename, "rb");

    if (!f)
    {
        fprintf(stderr, "Failed to open file %s\n", filename);
        return FILE_HANDLING_FAILURE;
    }

    *file = f;

    return FILE_HANDLING_SUCCESS;
}

int writefile_binary(FILE **file, char *filename)
{
    FILE *f = fopen(filename, "wb");

    if (!f)
    {
        fprintf(stderr, "Failed to open file %s\n", filename);
        return FILE_HANDLING_FAILURE;
    }

    *file = f;

    return FILE_HANDLING_SUCCESS;
}
