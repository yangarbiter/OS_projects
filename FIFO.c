#include"util.h"
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/wait.h>

#define RUN(time) \
		{ volatile unsigned long j; for(j=0;j<time;j++){ \
		{ volatile unsigned long i; for(i=0;i<1000000UL;i++); } \
													   }}

void FIFO(Process* process){
	int i;
	time_t s_start, s_end;
	long ns_start, ns_end;
	unsigned long now=0;
	pid_t* pid = (pid_t*)malloc(sizeof(pid_t) * process->numOfProc);

	for(i=0; i<process->numOfProc; i++){
		if(now-(process->R[i]) > 0)
			RUN(now-(process->R[i]));
		now = process->R[i] + process->T[i];
		gettime(&s_start, &ns_start);
		pid[i] = fork();
		if(pid[i] < 0){
			printf("fork fail\n");
		}else if(pid[i] == 0){
			RUN(process->T[i]);
			return;
		}else{
			char s[50];
			wait(NULL);
			
			gettime(&s_end, &ns_end);
			snprintf(s, 50, "%d %d.%ld %d.%ld\n", pid[i],
							(int)s_start, ns_start, (int)s_end, ns_end);
			printkk(s);

			printf("%s %d\n", process->N[i], pid[i]);
		}
	}
	
}
