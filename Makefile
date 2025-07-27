CC = gcc
CFLAGS = -Wall -Wextra -std=c99
SRCDIR = src
OBJDIR = build

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

renzip: $(OBJS)
	$(CC) $(CFLAGS) -o $(OBJDIR)/$@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) renzip

