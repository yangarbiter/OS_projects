
CC=gcc

all: main.c FIFO.c RR.c SJF.c
	$(CC) main.c FIFO.c RR.c SJF.c -o scheduler
