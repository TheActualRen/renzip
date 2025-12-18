#include "footer.h"
#include "tables/crc32_table.h"

uint32_t update_crc(uint32_t crc, uint8_t *buf, size_t len)
{
    uint32_t c = crc ^ 0xFFFFFFFF;

    for (size_t n = 0; n < len; n++)
    {
        c = table[(c ^ buf[n]) & 0xFF] ^ (c >> 8);
    }

    return c ^ 0xFFFFFFFF;
}

GZIP_FOOTER_STATUS write_gzip_footer(FILE *output_file, uint32_t crc, uint32_t isize)
{
    size_t bytes_written = fwrite(&crc, sizeof(uint32_t), 1, output_file);

    if (bytes_written != 1)
    {
        return GZIP_CRC_WRITE_FAILURE;
    }

    bytes_written = fwrite(&isize, sizeof(uint32_t), 1, output_file);

    if (bytes_written != 1)
    {
        return GZIP_ISIZE_WRITE_FAILURE;
    }

    return GZIP_FOOTER_SUCCESS;
}
