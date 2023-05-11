CC = gcc
CFLAGS = -Wall -g -I$(CURDIR)

all: program

program: main.c utils.c utils.h
	$(CC) $(CFLAGS) -o program main.c utils.c

run: program
	./program

run-file: program
	./program ./test_data/test.txt

run-dir: program
	./program ./test_data

clean:
	rm -f program