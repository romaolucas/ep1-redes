CC = gcc

.PHONY: all
all: servidor

servidor: imap_servidor.c
	$(CC) -o $@ $^

clean:
	rm servidor
