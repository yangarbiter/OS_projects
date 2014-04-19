CC=gcc
CFLAG=-Wall -std=c99

all: main.c FIFO.c RR.c SJF.c
	$(CC) $(CFLAG) main.c FIFO.c RR.c SJF.c -o scheduler

test: main.c util.h
	$(CC) $(CFLAG) main.c -o main
