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

        B0_STATUS status = blocktype0_encoding(input_file, output_file);

        switch (status)
        {
            case B0_HEADER_FAILURE:
                fprintf(stderr, "Error: Could not write gzip header.\n");
                break;

            case B0_INVALID_INPUT:
                fprintf(stderr, "Error: An invalid input has been provided. Ensure "
                                "the data provided does not exceed 64KB.\n");
                break;

            case B0_MALLOC_FAILURE:
                fprintf(stderr, "Error: Could not allocate memory for input buffer.\n");
                break;

            case B0_READ_FAILURE:
                fprintf(stderr, "Error: ");
                break;

            case B0_WRITING_FAILURE:
                fprintf(stderr, "Error: Could not write compressed data to file. Check bitwriter.\n");
                break;

            case B0_FOOTER_FAILURE:
                fprintf(stderr, "Error: Could not write gzip footer.\n");
                break;

            default:
                printf("Block Type 0 Compression Successful.\n");
        }
    }

    else if (strcmp(argv[1], "-e") == 0 && strcmp(argv[2], "-1") == 0)
    {
        FILE *input_file, *output_file;

        readfile_binary(&input_file, argv[3]);
        writefile_binary(&output_file, argv[4]);

        B1_STATUS status = blocktype1_encoding(input_file, output_file);

        switch (status)
        {
            case B1_HEADER_FAILURE:
                fprintf(stderr, "Error: Could not write gzip header.\n");
                break;

            case B1_MALLOC_FAILURE:
                fprintf(stderr, "Error: Could not allocate memory for input buffer.\n");
                break;

            case B1_READ_FAILURE:
                fprintf(stderr, "Error: ");
                break;

            case B1_WRITING_FAILURE:
                fprintf(stderr, "Error: Could not write compressed data to file. Check bitwriter.\n");
                break;

            case B1_FOOTER_FAILURE:
                fprintf(stderr, "Error: Could not write gzip footer.\n");
                break;

            default:
                printf("Block Type 1 Compression Successful.\n");
        }
    }

    else if (strcmp(argv[1], "-e") == 0 && strcmp(argv[2], "-2") == 0)
    {
        FILE *input_file, *output_file;

        readfile_binary(&input_file, argv[3]);
        writefile_binary(&output_file, argv[4]);

        B2_STATUS status = blocktype2_encoding(input_file, output_file);

        switch (status)
        {
            case B2_HEADER_FAILURE:
                fprintf(stderr, "Error: Could not write gzip header.\n");
                break;

            case B2_MALLOC_FAILURE:
                fprintf(stderr, "Error: Could not allocate memory for input buffer.\n");
                break;

            case B2_READ_FAILURE:
                fprintf(stderr, "Error: ");
                break;

            case B2_WRITING_FAILURE:
                fprintf(stderr, "Error: Could not write compressed data to file. Check bitwriter.\n");
                break;

            case B2_FOOTER_FAILURE:
                fprintf(stderr, "Error: Could not write gzip footer.\n");
                break;

            default:
                printf("Block Type 2 Compression Successful.\n");
        }
    }

    else if (strcmp(argv[1], "-d") == 0 && strcmp(argv[2], "-0") == 0)
    {
        FILE *input_file, *output_file;

        readfile_binary(&input_file, argv[3]);
        writefile_binary(&output_file, argv[4]);

        blocktype0_decoding(input_file, output_file);
        printf("Block Type 0 Decoding\n");
    }

    else if (strcmp(argv[1], "-d") == 0 && strcmp(argv[2], "-1") == 0)
    {
        FILE *input_file, *output_file;

        readfile_binary(&input_file, argv[3]);
        writefile_binary(&output_file, argv[4]);

        // blocktype1_decoding(input_file, output_file);
        printf("Block Type 1 Decoding\n");
    }

    else
    {
        FILE *input_file, *output_file;

        readfile_binary(&input_file, argv[3]);
        writefile_binary(&output_file, argv[4]);

        // blocktype2_decoding(input_file, output_file);
        printf("Block Type 2 Decoding\n");
    }

    return EXIT_SUCCESS;
}
