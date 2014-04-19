#include <time.h>
#include "util.h"

#define __NR_gettime 315
#define __NR_printkk 316

int gettime (time_t *s, long *ns) {
	return syscall (__NR_gettime, s, ns);
}

int printkk (char *s) {
	return syscall (__NR_printkk, s);
}
