CC = gcc
CFLAGS = -Wall

all: mini-shell

mini-shell: main.o shell.o
	$(CC) $(CFLAGS) -o mini-shell main.o shell.o

main.o: main.c shell.h
	$(CC) $(CFLAGS) -c main.c

shell.o: shell.c shell.h
	$(CC) $(CFLAGS) -c shell.c

clean:
	rm -f *.o mini-shell

