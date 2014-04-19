#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/time.h>

asmlinkage int sys_gettime(time_t *s, long *ns)
{
	struct timespec time;
	printk("gettime is invoked!\n");
	getnstimeofday(&time);
	*s = time.tv_sec;
	*ns = time.tv_nsec;
	return 2;
}
