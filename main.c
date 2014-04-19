#include<stdio.h>
#include<string.h>
#include"util.h"

int main (int argc, char *argv[]){
	int S, i;
	Process *proc = malloc(sizeof(Process));
	scanf("%s", S);
	scanf("%d", proc->numOfProc);

	proc->procs = malloc(sizeof(int*) * proc->numOfProc);
	for(i=0; i<proc->numOfProc; i++){
		proc->process[i] = malloc(sizeof(int) * 3);
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
