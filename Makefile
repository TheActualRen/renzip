EXE := renzip
CC  := gcc

# CFLAGS := -Wall -Wextra -std=c99 -O3 -MMD -MP
CFLAGS := -Wall -Wextra -std=c99 -O0 -g -fsanitize=address,undefined

SRCS :=
SRCS += main.c
SRCS += file_handling.c

SRCS += tables/crc32_table.c
SRCS += tables/fixed_huffman_tables.c

SRCS += header.c
SRCS += footer.c

SRCS += bitwriter.c
SRCS += bitreader.c

SRCS += lz77.c
SRCS += huffman_fixed.c
SRCS += huffman_dynamic.c
SRCS += queue.c
SRCS += rle.c

SRCS += blocktype0.c
SRCS += blocktype1.c
SRCS += blocktype2.c

OBJS := $(SRCS:.c=.o)
DEPS := $(OBJS:.o=.d)

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -f $(EXE) $(OBJS) $(DEPS)

.PHONY: all clean
