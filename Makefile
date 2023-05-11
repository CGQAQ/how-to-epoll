CC = gcc
CFLAGS = -Wall -g -I$(CURDIR)

all: program

program: main.c utils.c utils.h
	$(CC) $(CFLAGS) -o program main.c utils.c

run: program
	./program

clean:
	rm -f program