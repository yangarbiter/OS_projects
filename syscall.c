#define _GNU_SOURCE
#include <time.h>
#include "util.h"
#include <sys/syscall.h>
#include <unistd.h>

#define __NR_gettime 314
#define __NR_printkk 315

int gettime (time_t *s, long *ns) {
	return syscall (__NR_gettime, s, ns);
}

int printkk (char *s) {
	return syscall (__NR_printkk, s);
}
