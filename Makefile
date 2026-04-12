CC = gcc
CFLAGS = -Wall -I.

SRCS = main.c libs/memory.c libs/string.c libs/math.c libs/screen.c libs/keyboard.c
OBJS = $(SRCS:.c=.o)
EXEC = tutor

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJS) $(EXEC)
