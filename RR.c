#define _GNU_SOURCE
#include "util.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <sys/time.h>
#include <time.h>
#include <sys/wait.h>
struct Pid_start_time{
	time_t s_start, ns_start;
}pid_start_time[70000];

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
			//struct timespec fin;
			//clock_gettime(CLOCK_REALTIME, &fin);
			//printf("pid %d finished time: %lld.%.9ld\n", getpid(), (long long)fin.tv_sec, fin.tv_nsec);
			exit(0);
		}


		//設定只能一個CPU
		cpu_set_t cmask;
		CPU_ZERO(&cmask);
		CPU_SET(0, &cmask);
		sched_setaffinity(pid, sizeof(cpu_set_t), &cmask);
		printf("%s %d\n", p->N[p_index], pid);
		gettime(&pid_start_time[pid].s_start, &pid_start_time[pid].ns_start);
		//struct timespec st;
		//clock_gettime(CLOCK_REALTIME, &st);
		//printf("pid %d start time: %lld.%.9ld\n", pid, (long long)st.tv_sec, st.tv_nsec);
		last = p->R[p_index];
	}

	time_t s_end;
	long ns_end;
	for(int i = 0; i < p->numOfProc; i++){
		int cpid = wait(NULL);
		gettime(&s_end, &ns_end);
		char s[50];
		snprintf(s, 50, "%d %d.%ld %d.%ld\n", cpid,
				(int)pid_start_time[cpid].s_start, pid_start_time[cpid].ns_start, (int)s_end, ns_end);
		printkk(s);
	}
	exit(0);
}
