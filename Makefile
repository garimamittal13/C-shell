CC = gcc
CFLAGS = -Wall -I./src

SRCS = src/main.c src/prompt.c src/inputhandler.c src/commandhandler.c src/color.c src/log.c src/proclore.c src/seek.c src/myshrc.c src/neonate.c src/iman.c
OBJS = $(SRCS:.c=.o)

EXEC = my_shell

.PHONY: all clean

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS) -lcurl  # ✅ Link libcurl here

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(EXEC) $(OBJS)
