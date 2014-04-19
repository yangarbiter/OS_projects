<<<<<<< HEAD

CC=gcc

all: main.c FIFO.c RR.c SJF.c
	$(CC) main.c FIFO.c RR.c SJF.c -o scheduler
=======
all: main.c util.h
	gcc -Wall main.c -o main
>>>>>>> 4bea508840c596c45fd8aba8d683eed0b56ade19
