CC=gcc
CFLAGS=-I. -ansi -pedantic -Wall

all: main.o utils.o
	$(CC) $(CFLAGS) -o run main.o utils.o

main.o: main.c utils.h
	$(CC) $(CFLAGS) -c main.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

clean:
	rm *.x *.o run
