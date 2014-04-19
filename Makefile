all: main.c util.h RR.o
	gcc -std=c99 -Wall RR.o main.c -o main

RR.o: RR.c RR.h
	gcc -std=c99 -lpthread -Wall -c RR.c
