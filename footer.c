#include "footer.h"
#include "tools/table.h"

uint32_t update_crc(uint32_t crc, uint8_t *buf, int len)
{
    uint32_t c = crc ^ 0xFFFFFFFF;

    for (int n = 0; n < len; n++)
    {
        c = table[(c ^ buf[n]) & 0xFF] ^ (c >> 8);
    }

    return c ^ 0xFFFFFFFF;
}

int write_gzip_footer(FILE *output_file, uint32_t crc, uint32_t isize)
{
    if (!fwrite(&crc, sizeof(uint32_t), 1, output_file))
    {
        fprintf(stderr, "Error writing CRC32 in the gzip footer\n");
        return GZIP_FOOTER_FAILURE;
    }

    if (!fwrite(&isize, sizeof(uint32_t), 1, output_file))
    {
        fprintf(stderr, "Error writing ISIZE in the gzip footer\n");
        return GZIP_FOOTER_FAILURE;
    }

    return GZIP_FOOTER_SUCCESS;
}
