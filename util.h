
#ifndef UTIL
#define UTIL

typedef struct _Proces{
	int numOfProc;
	int **procs;
}Proces

void FIFO(Proces* proces);
void RR(Proces* proces);
void SJF(Proces* proces);

#endif
