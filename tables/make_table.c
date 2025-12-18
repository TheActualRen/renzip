#include <stdio.h>
#include <stdint.h>

void write_crc_table(uint32_t *crc_table)
{
    FILE *file = fopen("crc32_table.c", "wa");

    if (!file)
    {
        perror("Error opening file");
        return;
    }

    fprintf(file, "#include <stdint.h>\n\n");
    fprintf(file, "uint32_t table[256] = {\n    ");

    for (int n = 0; n < 256; n++)
    {
        if (n == 0 || n % 6 != 0)
        {
            fprintf(file, "0x%.8X, ", crc_table[n]);
        }

        else 
        {
            fprintf(file, "\n    0x%.8X, ", crc_table[n]);
        }
    }

    fprintf(file, "\n};");
    fclose(file);
}

void make_crc_table(void)
{
    const uint32_t polynomial = 0xEDB88320;

    uint32_t crc_table[256];
    uint32_t c;

    for (int n = 0; n < 256; n++)
    {
        c = (uint32_t) n;

        for (int k = 0; k < 8; k++)
        {
            if (c & 1)
            {
                c = polynomial ^ (c >> 1);
            }

            else 
            {
                c >>= 1;
            }
        }
        // printf("CRC: 0x%.8X\n", c);
        crc_table[n] = c;
    }

    write_crc_table(crc_table);
}

int main(void)
{
    make_crc_table();
    return 0;
}
