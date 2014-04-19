#include"util.h"
#include<unistd.h>

#define RUN(time) \
		{ volatile unsigned long i; for(i=0;i<time;i++); }

void FIFO(Process* process){
	int i;
	unsigned long now=0;
	pid_t* pid = (pid_t*)malloc(sizeof(pid_t) * process->numOfProcess);

	for(i=0; i<process->numOfProcess; i++){
		if(now-(process->R[i])) > 0)
			RUN(now-(process->R[i]));
		now = process->R[i];
		pid[i] = fork();
		if(pid[i] < 0){
			printf("fork fail\n");
		}else if(pid[i] == 0){
			RUN(process->T[i]);
		}else{
			wait(NULL);
			printf("%s %d", process->N[i], pid[i]);
		}
	}
	
}
