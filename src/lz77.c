#include "../include/lz77.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define WINDOW_SIZE 32768
#define MAX_MATCH 258
#define MIN_MATCH 3

static bool append_token(LZ77TokenList* list, LZ77Token token) {
	if (list->count >= list->capacity) {
		size_t new_cap = list->capacity ? list->capacity * 2 : 128;
		LZ77Token* new_tokens = realloc(list->tokens, new_cap * sizeof(LZ77Token));

		if (!new_tokens){
			fprintf(stderr, "Error: failed to reallocate memory for new tokens\n");
			return false;
		} 
		list->tokens = new_tokens;
		list->capacity = new_cap;

	}
	list->tokens[list->count++] = token;
	return true;
}

void lz77_free_tokens(LZ77TokenList* list) {
	if (list && list->tokens) {
		free(list->tokens);
		list->tokens = NULL;
		list->count = 0;
		list->capacity = 0;
	}
}

void lz77_compress(const uint8_t* input, size_t input_len, 
                   LZ77TokenList* out_tokens) {
    memset(out_tokens, 0, sizeof(*out_tokens));
    size_t pos = 0;

    while (pos < input_len) {
        size_t best_len = 0;
        size_t best_dist = 0;
        size_t start = (pos > WINDOW_SIZE) ? pos - WINDOW_SIZE : 0;

        for (size_t search = start; search < pos; search++) {
            size_t match_len = 0;
            while (match_len < MAX_MATCH &&
                   pos + match_len < input_len &&
                   input[search + match_len] == input[pos + match_len]) {
                match_len++;
            }

            if (match_len >= MIN_MATCH && match_len > best_len) {
                best_len = match_len;
                best_dist = pos - search;
                if (match_len == MAX_MATCH) break;
            }
        }

        if (best_len >= MIN_MATCH) {
            LZ77Token token = {
                .type = LZ77_MATCH,
                .match = { .len = best_len, .dist = best_dist }
            };
            append_token(out_tokens, token);
            pos += best_len;
        } else {
            LZ77Token token = {
                .type = LZ77_LITERAL,
                .literal = input[pos]
            };
            append_token(out_tokens, token);
            pos++;
        }
    }
}
