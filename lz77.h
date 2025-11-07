#ifndef LZ77_H
#define LZ77_H

#include <stdint.h>
#include <stddef.h>

#define LZ77_SUCCESS 0
#define LZ77_REALLOC_FAILURE 1
#define LZ77_FREEING_FAILURE 2
#define LZ77_APPENDING_TOKEN_FAILURE 3

#define WINDOW_SIZE 32768
#define MAX_MATCH 258
#define MIN_MATCH 3

typedef enum
{
    LITERAL,
    LENGTH_DIST_PAIR,
} TokenType;

typedef struct
{
    TokenType type;

    union
    {
        uint8_t literal;

        struct
        {
            uint16_t len;
            uint16_t dist;
        } LengthDistPair;
    };

} LZ77Token;

typedef struct
{
    LZ77Token *tokens;
    size_t count;
    size_t capacity;

} LZ77TokenList;

int lz77_compress(const uint8_t *input_buf, size_t input_len,
                   LZ77TokenList *output_tokens);

int lz77_free_tokens(LZ77TokenList *list);

#endif
