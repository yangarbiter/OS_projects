#define _POSIX_SOURCE
#define _XOPEN_SOURCE
#include"util.h"
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<signal.h>

#define RUN(time) \
		{ volatile unsigned long j; for(j=0;j<time;j++){ \
		{ volatile unsigned long i; for(i=0;i<1000000UL;i++); }}}

volatile sig_atomic_t current;
static time_t s_start, s_end;
static long ns_start, ns_end;
static pid_t* pid; 

void sigchld_handler(int param){
	char s[50];
	wait(NULL);
	//printf("sigchld!");

	gettime(&s_end, &ns_end);
	snprintf(s, 50, "%d %d.%ld %d.%ld\n", pid[current],
					(int)s_start, ns_start, (int)s_end, ns_end);
	printkk(s);

// activate the next child
	current++;
	kill(pid[current], SIGUSR1);
	gettime(&s_start, &ns_start);
}

void FIFO(Process* process){
	int i;
	unsigned long now=0;
	struct sigaction act;
	nice(-19);

	act.sa_handler = sigchld_handler;
	sigemptyset (&act.sa_mask);
	sigaddset (&act.sa_mask, SIGCHLD);
	act.sa_flags = 0;
	sigaction (SIGCHLD, &act, NULL);

	pid = (pid_t*)malloc(sizeof(pid_t) * process->numOfProc);

	current = 0;

	for(i=0; i<process->numOfProc; i++){
		if((process->R[i])-now > 0)
			RUN((process->R[i])-now);
		now = process->R[i];

		pid[i] = fork();
		if(i==0)gettime(&s_start, &ns_start);

		if(pid[i] < 0){
			printf("fork fail\n");
		}else if(pid[i] == 0){
			if(i!=0)nice(19);

			RUN(process->T[i]);
			printf("pid %d ended\n", getpid());
			kill(getppid(), SIGCHLD);
			exit(0);
		}else{
			printf("%s %d\n", process->N[i], pid[i]);
		}
	}
	while(current < process->numOfProc);
	
}
