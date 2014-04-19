all: main.c util.h
	gcc -Wall main.c FIFO.c -o main
