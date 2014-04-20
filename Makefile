CC = gcc
CFLAG = -Wall -std=c99 -pthread -g
OBJS = FIFO.o RR.o SJF.o syscall.o

all: main.c $(OBJS)
	$(CC) $(CFLAG) $^ -o scheduler

$(OBJS): %.o: %.c
	$(CC) $(CFLAG) -c $*.c -o $*.o 

clean:
	rm -f ./*.o

test: main.c util.h
	$(CC) $(CFLAG) main.c -o main
