#ifndef UTIL
#define UTIL

#include <time.h>

typedef struct _Process{
	int numOfProc;
	char **N;
	unsigned int *R, *T;
}Process;

void FIFO(Process* process);
void RR(Process* process);
void SJF(Process* process);

int gettime (time_t *s, long *ns) ;
int printkk (char *s) ;

#endif
