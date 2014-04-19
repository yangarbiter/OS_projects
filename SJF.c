#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "util.h"

#define WAIT \
	{volatile unsigned int i; for (i = 0 ; i < 1000000UL ; i++) ;}

/* return -1 if no next */
int pickNext (const Process *process, const int *finish, unsigned int time) {
	int i, p;

	p = -1;
	for (i = 0 ; i < process->numOfProc ; i++) {
		if (!finish[i] && process->R[i] <= time) {
			if (p == -1) {
				p = i;
			}
			else if (process->T[i] < process->T[p]) {
				p = i;
			}
		}
	}
	if (p != -1) { /* Ready queue is not empty */
		return p;
	}

	/* Ready queue is empty */
	for (i = 0 ; i < process->numOfProc ; i++) {
		if (!finish[i]) {
			if (p == -1) {
				p = i;
			}
			else if (process->R[i] == process->R[p] && process->T[i] < process->T[p]) {
				p = i;
			}
			else if (process->R[i] > process->R[p]) {
				break;
			}
		}
	}

	return p;
}

void SJF (Process *process) {
	unsigned int t;
	int *finish;
	pid_t *pid;
	int w, next;

	time_t st_s, ed_s;
	long st_ns, ed_ns;

	char msg[1024];

	t = 0;
	finish = (int*) calloc (process->numOfProc, sizeof (int));
	pid = (pid_t*) malloc (process->numOfProc * sizeof (pid_t));
	while (1) {
		next = pickNext (process, finish, t);

		if (next == -1) {
			break;
		}
		else if (process->R[next] <= t) {
			pid[next] = fork ();
			if (pid[next] == 0) {
				for (w = 0 ; w < process->T[next] ; w++) {
					WAIT;
				}

				exit (0);
			}
			else if (pid[next] > 0) {
				printf ("%s %d\n", process->N[next], (int) pid[next]);
				gettime (&st_s, &st_ns);
				waitpid (pid[next], NULL, 0);
				gettime (&ed_s, &ed_ns);

				snprintf (msg, sizeof (msg), "%d %u.%ld %u.%ld\n", (int) pid[next], (unsigned int) st_s, st_ns, (unsigned int) ed_s, ed_ns);
				printkk (msg);

				t += process->T[next];
				finish[next] = 1;
			}
			else {
				fprintf (stderr, "Error fork new process\n");

				exit (1);
			}
		}
		else {
			for (w = 0 ; w < process->R[next] - t ; w++) {
				WAIT;
			}

			t += w;
		}
	}
}
