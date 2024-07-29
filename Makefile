CC = gcc
CFLAGS = -Wall -Wextra

build:
	$(CC) $(CFLAGS) -o .build/server src/server.c