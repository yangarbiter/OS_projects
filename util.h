#ifndef UTIL
#define UTIL

typedef struct _Process{
	int numOfProc;
	char **N;
	unsigned int *R, *T;
}Process;

void FIFO(Process* process);
void RR(Process* process);
void SJF(Process* process);

#endif
