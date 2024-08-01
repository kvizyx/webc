CC = gcc
CFLAGS = -Wall -Wextra

build:
	$(CC) $(CFLAGS) src/server.c src/request.c src/main.c -o .build/server