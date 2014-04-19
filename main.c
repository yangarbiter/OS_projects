#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "RR.h"

void swapCharPointer (char **c1, char **c2) {
	char *tmp;
	tmp = *c1;
	*c1 = *c2;
	*c2 = tmp;
}

void swapUnsignedInt (unsigned int *c1, unsigned int *c2) {
	unsigned int tmp;
	tmp = *c1;
	*c1 = *c2;
	*c2 = tmp;
}

int main (int argc, char *argv[]){
	char S[1024];
	int i, j;
	Process *proc = (Process*) malloc(sizeof(Process));

	scanf("%s", S);
	scanf("%d", &proc->numOfProc);

	proc->N = (char**) malloc (proc->numOfProc * sizeof (char*));
	proc->R = (unsigned int*) malloc (proc->numOfProc * sizeof (unsigned int));
	proc->T = (unsigned int*) malloc (proc->numOfProc * sizeof (unsigned int));
	for(i=0; i<proc->numOfProc; i++){
		proc->N[i] = (char*) malloc (32 * sizeof (char));
		scanf ("%s %d %d", proc->N[i], &proc->R[i], &proc->T[i]);
	}

	for (i = proc->numOfProc - 1 ; i > 0 ; i--) {
		for (j = 0 ; j < i ; j++) {
			if (proc->R[j] > proc->R[j + 1] || (proc->R[j] == proc->R[j + 1] && proc->T[j] > proc->T[j + 1])) {
				swapCharPointer (&proc->N[j], &proc->N[j + 1]);
				swapUnsignedInt (&proc->R[j], &proc->R[j + 1]);
				swapUnsignedInt (&proc->T[j], &proc->T[j + 1]);
			}
		}
	}
	
	if(!strcmp(S, "FIFO")){
		FIFO(proc);
	}else if(!strcmp(S, "RR")){
		RR(proc);
	}else{
		SJF(proc);
	}

	return 0;
}


void FIFO(Process* process){}
void SJF(Process* process){}
