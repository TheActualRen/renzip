#include "lz77.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int append_token(LZ77TokenList *list, LZ77Token token)
{
    if (list->count >= list->capacity)
    {
        size_t new_cap;

        if (list->capacity)
        {
            new_cap = list->capacity * 2;
        }

        else
        {
            new_cap = 128;
        }

        LZ77Token *new_tokens = realloc(list->tokens, new_cap * sizeof(LZ77Token));
        
        if (!new_tokens)
        {
            fprintf(stderr, "Failed to reallocate memory for new tokens\n");
            return LZ77_REALLOC_FAILURE;
        }

        list->tokens = new_tokens;
        list->capacity = new_cap;
    }

    list->tokens[list->count] = token;
    list->count++;

    return LZ77_SUCCESS;
}

int lz77_compress(const uint8_t *input_buf, size_t input_len,
                   LZ77TokenList *output_tokens)
{
    memset(output_tokens, 0, sizeof(*output_tokens));
    size_t pos = 0;

    while (pos < input_len)
    {
        size_t best_len = 0;
        size_t best_dist = 0;

        size_t start;

        if (pos > WINDOW_SIZE)
        {
            start = pos - WINDOW_SIZE;
        }

        else
        {
            start = 0;
        }

        for (size_t search = start; search < pos; search++)
        {
            size_t match_len = 0;

            while (match_len < MAX_MATCH && pos + match_len < input_len &&
                   input_buf[search + match_len] == input_buf[pos + match_len])
            {
                match_len++;
            }

            if (match_len >= MIN_MATCH && match_len > best_len)
            {
                best_len = match_len;
                best_dist = pos - search;

                if (match_len == MAX_MATCH) break;
            }
        }

        if (best_len >= MIN_MATCH)
        {
            LZ77Token token = 
            {
                .type = LENGTH_DIST_PAIR,
                .LengthDistPair = { .len = best_len, .dist = best_dist }
            };

            if (append_token(output_tokens, token) != LZ77_SUCCESS)
            {
                fprintf(stderr, "Error Appending token\n");
                return LZ77_APPENDING_TOKEN_FAILURE;
            }

            pos += best_len;
        }

        else 
        {
            LZ77Token token = 
            {
                .type = LITERAL,
                .literal = input_buf[pos]
            };

            if (append_token(output_tokens, token) != LZ77_SUCCESS)
            {
                fprintf(stderr, "Error Appending token\n");
                return LZ77_APPENDING_TOKEN_FAILURE;
            }

            pos++;
        }
    }

    return LZ77_SUCCESS;
}

int lz77_free_tokens(LZ77TokenList *list)
{
    if (!list || !list->tokens)
    {
        fprintf(stderr, "Error freeing lz77 tokens\n");
        return LZ77_FREEING_FAILURE;
    }

    free(list->tokens);
    list->tokens = NULL;
    list->count = 0;
    list->capacity = 0;
    return LZ77_SUCCESS;
}
