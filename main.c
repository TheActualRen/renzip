#include "file_handling.h"
#include "blocktype0.h"
#include "blocktype1.h"
#include "blocktype2.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    for (int i = 0; i < argc; i++)
    {
        printf("argv[%d] = %s\n", i, argv[i]);
    }

    if (argc != 5)
    {
        fprintf(
            stderr,
            "Not enough arguments were provided. 4 are needed. %d were given\n",
            argc - 1);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "-e") != 0 && strcmp(argv[1], "-d") != 0)
    {
        fprintf(
            stderr,
            "Not a valid cmd line argument. argv[0] must equal '-e' or '-d'\n");
        return EXIT_FAILURE;
    }

    if (strcmp(argv[2], "-0") != 0 && strcmp(argv[2], "-1") != 0 &&
        strcmp(argv[2], "-2") != 0)
    {
        fprintf(stderr, "Not a valid cmd line argument. argv[1] must equal "
                        "'-0', '-1' or '-2'\n");
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "-e") == 0 && strcmp(argv[2], "-0") == 0)
    {
        FILE *input_file, *output_file;

        readfile_binary(&input_file, argv[3]);
        writefile_binary(&output_file, argv[4]);

        blocktype0_encoding(input_file, output_file);
        printf("Block Type 0 Encoding\n");
    }

    else if (strcmp(argv[1], "-e") == 0 && strcmp(argv[2], "-1") == 0)
    {
        FILE *input_file, *output_file;

        readfile_binary(&input_file, argv[3]);
        writefile_binary(&output_file, argv[4]);

        blocktype1_encoding(input_file, output_file);
        printf("Block Type 1 Encoding\n");
    }

    else if (strcmp(argv[1], "-e") == 0 && strcmp(argv[2], "-2") == 0)
    {
        FILE *input_file, *output_file;

        readfile_binary(&input_file, argv[3]);
        writefile_binary(&output_file, argv[4]);

        blocktype2_encoding(input_file, output_file);

        printf("Block Type 2 Encoding\n");
    }

    else if (strcmp(argv[1], "-d") == 0 && strcmp(argv[2], "-0") == 0)
    {
        printf("Block Type 0 Decoding\n");
    }

    else if (strcmp(argv[1], "-d") == 0 && strcmp(argv[2], "-1") == 0)
    {
        printf("Block Type 1 Decoding\n");
    }

    else
    {
        printf("Block Type 2 Decoding\n");
    }

    return EXIT_SUCCESS;
}
