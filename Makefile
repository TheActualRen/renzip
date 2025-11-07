EXE = renzip 

SRCS =
SRCS += main.c
SRCS += file_handling.c
SRCS += header.c
SRCS += footer.c
SRCS += bitwriter.c
SRCS += blocktype0.c
SRCS += tools/table.c
SRCS += huffman_fixed.c
SRCS += lz77.c
SRCS += blocktype1.c
SRCS += queue.c
SRCS += huffman_dynamic.c
SRCS += blocktype2.c
SRCS += rle.c


LIBS =

OBJS = $(patsubst %.c,%.o,$(filter %.c,${SRCS}))

DEPS = $(patsubst %.o,%.d,${OBJS})

CC = gcc

CFLAGS =
CFLAGS +=	-MMD
CFLAGS +=	-O3
CFLAGS +=	-Wall
CFLAGS +=	-Werror
CFLAGS +=	-Wextra
CFLAGS +=	-std=gnu2x

LD = gcc

LDFLAGS	=

all:	${EXE}

.PHONY:	all

clean:
	rm -f ${OBJS}
	rm -f ${DEPS}

.PHONY:	clean 

distclean:	clean
	rm -f ${EXE}

.PHONY:	distclean

${EXE}: ${OBJS}
	${LD} ${LDFLAGS} $^ ${LIBS} -o $@

%.o:	%.c
	${CC} ${CFLAGS} -c $< -o $@

-include ${DEPS}
