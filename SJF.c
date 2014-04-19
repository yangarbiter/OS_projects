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

sig_atomic_t pick, complete;

void SIGCHLDHandler (int param) {
	pick = 1;
	complete++;
}

typedef struct {
	Process *process;
} HandleProcessArgs;

void* handleProcess (void *arg) {
	HandleProcessArgs *_arg = (HandleProcessArgs*) arg;
	const Process *process = _arg->process;
	pid_t pid;
	int p;
	time_t s;
	long ns;

	char msg[1024];

	int i;

	pick = 0;
	signal (SIGCHLD, SIGCHLDHandler);

	while (complete < process->numOfProc) {
		while (pick == 0) ;
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

		pick = 0;
	}

	return NULL;
}

void SJF (Process *process) {
	unsigned int t;
	int i;

	pthread_t tid;

	complete = 0;

	pthread_create (&tid, NULL, handleProcess, process);

	t = 0;
	procInfo = (ProcessAccounting*) malloc (process->numOfProc * sizeof (ProcessAccounting));
	for (i = 0 ; i < process->numOfProc ; i++) {
		procInfo[i].state = INITIAL;
		procInfo[i].pid   = 0;
		procInfo[i].st_s  = procInfo[i].ed_s  = 0;
		procInfo[i].st_ns = procInfo[i].ed_ns = 0;
	}
	while (complete < process->numOfProc) {
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
					procInfo[i].state = READY;
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
