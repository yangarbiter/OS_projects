CC = gcc
CFLAG = -Wall -std=c99

all: main.c FIFO.o RR.o SJF.o syscall.o
	$(CC) $(CFLAG) $^ -o scheduler

%.o: $*.c
	$(CC) $(CFLAG) -c $*.c -o $*.o

clean:
	rm -f ./*.o

test: main.c util.h
	$(CC) $(CFLAG) main.c -o main
