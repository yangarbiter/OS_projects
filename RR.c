#include "util.h"
#include <unistd.h>
#include <stdio.h>

void unit_time(int times){
	volatile unsigned long i;
	for(int t = 0; t < times; t++){
		for(i=0;i<1000000UL;i++);
	}
}

void RR(Process *p){
	int last = 0;
	for(int p_index = 0; p_index < p->numOfProc; p_index++){
		unit_time(p->R[p_index] - last);
		int pid = fork();
		if (pid == 0) {
			unit_time(p->T[p_index]);
		}
		else {
			printf("%s %d\n", p->N[p_index], pid);
		}
		last = p->R[p_index];
	}
}
