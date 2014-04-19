#define _GNU_SOURCE
#include "util.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <sys/time.h>
#include <time.h>

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
			struct timespec fin;
			clock_gettime(CLOCK_REALTIME, &fin);
			printf("pid %d finished time: %lld.%.9ld\n", getpid(), (long long)fin.tv_sec, fin.tv_nsec);
			exit(0);
		}


		//設定只能一個CPU
		cpu_set_t cmask;
		CPU_ZERO(&cmask);
		CPU_SET(0, &cmask);
		sched_setaffinity(pid, sizeof(cpu_set_t), &cmask);
		printf("%s %d\n", p->N[p_index], pid);

		struct timespec st;
		clock_gettime(CLOCK_REALTIME, &st);
		printf("pid %d start time: %lld.%.9ld\n", pid, (long long)st.tv_sec, st.tv_nsec);
		last = p->R[p_index];
	}
	exit(0);
}
