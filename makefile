CC=gcc
CFLAGS=-Wall -std=c99

all: osh

osh: osh.c
	$(CC) $(CFLAGS) -o osh osh.c

clean:
	rm -f osh

