#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

int main (int argc, char *argv[]){
	char S[1024];
	int i;
	Process *proc = (Process*) malloc(sizeof(Process));

	scanf("%s", S);
	scanf("%d", &proc->numOfProc);

	proc->procs = (int**) malloc(sizeof(int*) * proc->numOfProc);
	for(i=0; i<proc->numOfProc; i++){
		proc->procs[i] = (int*) malloc(sizeof(int) * 3);
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
