#define _POSIX_SOURCE
#define _XOPEN_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include "util.h"

#define WAIT \
	{volatile unsigned int i; for (i = 0 ; i < 1000000UL ; i++) ;}

typedef enum {
	INITIAL, READY, RUNNING, TERMINATE
} State;

typedef struct {
	State state;
	pid_t pid;
	time_t st_s, ed_s;
	long st_ns, ed_ns;
} ProcessAccounting;
ProcessAccounting *procInfo;

volatile sig_atomic_t pick;

void SIGCHLDHandler (int param) {
	pick++;
	puts ("catch SIGCHLD!!");
	printf ("%d\n", pick);
}

void* handleProcess (void *arg) {
	const Process *process = (const Process*) arg;
	pid_t pid;
	int p;
	time_t s;
	long ns;

	int complete = 0;

	char msg[1024];

	int i;

	signal (SIGCHLD, SIGCHLDHandler);

	while (complete < process->numOfProc) {
		while (pick == 0) ;
		printf ("pick = %d\n", pick);
		gettime (&s, &ns);
		pid = wait (NULL);

		for (p = 0 ; p < process->numOfProc ; p++) {
			if (procInfo[p].pid == pid) {
				break;
			}
		}

		procInfo[p].state = TERMINATE;
		procInfo[p].ed_s = s;
		procInfo[p].ed_ns = ns;

		snprintf (msg, sizeof (msg), "%d %u.%ld %u.%ld\n",
				  (int) procInfo[p].pid,
				  (unsigned int) procInfo[p].st_s, procInfo[p].st_ns,
				  (unsigned int) procInfo[p].ed_s, procInfo[p].ed_ns);
		printkk (msg);

		p = -1;
		for (i = 0 ; i < process->numOfProc ; i++) {
			if (procInfo[i].state == READY) {
				if (p == -1) {
					p = i;
				}
				else if (process->T[i] < process->T[p]) {
					p = i;
				}
			}
		}
		if (p != -1) {
			gettime (&procInfo[p].st_s, &procInfo[p].st_ns);
			kill (procInfo[p].pid, SIGUSR1);
			procInfo[p].state = RUNNING;
		}

		pick--;
		complete++;
	}

	return NULL;
}

void SJF (Process *process) {
	unsigned int t;
	int first = 1;
	int i;
	
	int start = 0;

	pthread_t tid;

	pthread_create (&tid, NULL, handleProcess, process);

	t = 0;
	procInfo = (ProcessAccounting*) malloc (process->numOfProc * sizeof (ProcessAccounting));
	for (i = 0 ; i < process->numOfProc ; i++) {
		procInfo[i].state = INITIAL;
		procInfo[i].pid   = 0;
		procInfo[i].st_s  = procInfo[i].ed_s  = 0;
		procInfo[i].st_ns = procInfo[i].ed_ns = 0;
	}
	while (start < process->numOfProc) {
		for (i = 0 ; i < process->numOfProc ; i++) {
			if (procInfo[i].state == INITIAL && process->R[i] <= t) {
				procInfo[i].pid = fork ();
				if (procInfo[i].pid == 0) {
					int w;

					nice (20);

					for (w = 0 ; w < process->T[i] ; w++) {
						WAIT;
					}

					exit (0);
				}
				else if (procInfo[i].pid > 0) {
					printf ("%s %d\n", process->N[i], (int) procInfo[i].pid);
					if (first == 1) {
						gettime (&procInfo[i].st_s, &procInfo[i].st_ns);
						kill (procInfo[i].pid, SIGUSR1);
						procInfo[i].state = RUNNING;
						first = 0;
					}
					else {
						procInfo[i].state = READY;
					}

					start++;
				}
				else {
					fprintf (stderr, "Error fork new process\n");

					exit (1);
				}
			}
		}

		WAIT;
		t += 1;
	}

	pthread_join (tid, NULL);

	free (procInfo);
}
