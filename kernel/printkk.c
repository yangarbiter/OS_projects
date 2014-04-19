#include <linux/linkage.h>
#include <linux/kernel.h>

asmlinkage int sys_printkk(char* s)
{
	printk("s");
	return 1;
}
