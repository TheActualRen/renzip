#ifndef LZ77_H
#define LZ77_H

#include <stddef.h>
#include <stdint.h>

#define WINDOW_SIZE 32768 // 32KB
#define MAX_MATCH 258
#define MIN_MATCH 3

typedef enum
{
    LZ77_SUCCESS = 0,
    LZ77_REALLOC_FAILURE,
    LZ77_FREEING_FAILURE,
    LZ77_APPEND_TOKEN_FAILURE,

} LZ77_STATUS;

typedef enum
{
    LZ77_LITERAL,
    LZ77_MATCH,

} LZ77TokenType;

typedef struct
{
    LZ77TokenType type;

    union
    {
        uint8_t literal;

        struct
        {
            uint16_t len;
            uint16_t dist;

        } match;
    };

} LZ77Token;

typedef struct
{
    LZ77Token *data;
    size_t count;
    size_t capacity;

} LZ77TokenList;

LZ77_STATUS lz77_compress(const uint8_t *input_buf, size_t input_len,
                          LZ77TokenList *output_tokens);

LZ77_STATUS lz77_free_tokens(LZ77TokenList *list);

void lz77_count_freq(const LZ77TokenList *tokens, uint32_t ll_freqs[286],
                     uint32_t dist_freqs[30]);

#endif
