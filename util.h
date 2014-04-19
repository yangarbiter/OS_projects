
#ifndef UTIL
#define UTIL

typedef struct _Process{
	int numOfProc;
	int **procs;
}Process;

void FIFO(Process* process);
void RR(Process* process);
void SJF(Process* process);

#endif
