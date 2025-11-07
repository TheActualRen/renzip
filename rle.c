#include "rle.h"

void rle_encode(const uint8_t *input_buf, size_t len, RLEItem *output_buf,
                size_t *output_len)
{
    size_t j = 0;
    size_t i = 0;

    while (i < len)
    {
        uint8_t curr_bit = input_buf[i];
        size_t run = 1;

        while (i + run < len && input_buf[i + run] == curr_bit && run < 138)
        {
            run++;
        }

        if (curr_bit == 0 && run >= 3)
        {
            size_t rem = run;

            while (rem >= 11)
            {
                size_t take;

                if (rem > 138)
                {
                    take = 138;
                }

                else
                {
                    take = rem;
                }

                if (j >= MAX_OUTPUT_CAP)
                    break;

                output_buf[j] = (RLEItem){.symbol = 18,
                                          .offset_bits = 7,
                                          .extra_value = (uint16_t)(take - 11)};
                j++;

                rem -= take;
            }

            if (rem >= 3)
            {
                if (j >= MAX_OUTPUT_CAP)
                    break;

                size_t take;

                if (rem <= 10)
                {
                    take = rem;
                }

                else
                {
                    take = 10;
                }

                output_buf[j] = (RLEItem){.symbol = 17,
                                          .offset_bits = 3,
                                          .extra_value = (uint16_t)(take - 3)};
                j++;

                rem -= take;
            }

            for (size_t k = 0; k < rem; k++)
            {
                if (j >= MAX_OUTPUT_CAP)
                    break;

                output_buf[j] =
                    (RLEItem){.symbol = 0, .offset_bits = 0, .extra_value = 0};

                j++;
            }
        }

        else if (run >= 4)
        {
            if (j < MAX_OUTPUT_CAP)
            {
                output_buf[j] = (RLEItem) {.symbol = curr_bit, .offset_bits =0, .extra_value = 0};
                j++;
            }

            size_t rem = run - 1;

            while (rem >= 3)
            {
                size_t rep;

                if (rem > 6)
                {
                    rep = 6;
                }

                else
                {
                    rep = rem;
                }

                if (j >= MAX_OUTPUT_CAP)
                    break;

                output_buf[j] = (RLEItem){.symbol = 16,
                                          .offset_bits = 2,
                                          .extra_value = (uint16_t)(rep - 3)};

                j++;

                rem -= rep;
            }

            for (size_t k = 0; k < rem; k++)
            {
                if (j >= MAX_OUTPUT_CAP)
                    break;

                output_buf[j] = (RLEItem){
                    .symbol = curr_bit, .offset_bits = 0, .extra_value = 0};
                j++;
            }
        }

        else
        {
            for (size_t k = 0; k < run; k++)
            {
                if (j >= MAX_OUTPUT_CAP)
                    break;

                output_buf[j] = (RLEItem){
                    .symbol = curr_bit, .offset_bits = 0, .extra_value = 0};

                j++;
            }
        }

        i += run;
    }

    *output_len = j;
}
