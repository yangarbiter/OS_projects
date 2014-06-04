#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/workqueue.h>
#include <linux/init.h>
#include <linux/inet.h>
#include <net/sock.h>
#include <net/tcp.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");

static loff_t my_llseek(struct file *filp, loff_t off, int whence);
static ssize_t my_read(struct file *filp, char __user *buff, size_t count, loff_t *offp);
static ssize_t my_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp);
static long my_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param);
static int my_open(struct inode *inode, struct file *file);
static int my_close(struct inode *inode, struct file *file);
static ssize_t driver_os_recv(struct socket *csock, char *buf, size_t size);
// static ssize_t driver_os_send(struct socket *csock, char *buf, size_t size);
// static *void my_mmap(struct file * filp, struct vm_area_struct *vma);

static struct file_operations driver_os_ops = {
	.owner = THIS_MODULE,
	.llseek = my_llseek,
	.read = my_read,
	.write = my_write,
	.unlocked_ioctl = my_ioctl,
	.open = my_open,
	.release = my_close,
	// .mmap = my_mmap
};

static dev_t devno;
static struct class *driver_os_cl;
static struct cdev driver_os_dev;
static struct workqueue_struct *wq;
DECLARE_WAIT_QUEUE_HEAD(driver_os_wait);
static struct socket *ssock;
static char sockbuf[4096];
volatile int flag; /* 1 for unread */
static int datalen;

static int __init initialize(void)
{
	int ret;

	// KERN_ERR => <3> 代表發生錯誤
	if((ret = alloc_chrdev_region(&devno, 0, 1, "driver_os_slave")) < 0){
		printk(KERN_ERR "alloc_chrdev_region returned %d\n", ret);
		return ret;
	}

	if((driver_os_cl = class_create(THIS_MODULE, "chardrv_slave")) == NULL){
		printk(KERN_ERR "class_create returned NULL\n");
		ret = -ENOMEM;
		goto class_create_failed;
	}

	// 以driver_os_cl這個類創建/dev/driver_os檔案
	if(device_create(driver_os_cl, NULL, devno, NULL, "driver_os_slave") == NULL){
		printk(KERN_ERR "device_create returned NULL\n");
		ret = -ENOMEM;
		goto device_create_failed;
	}

	cdev_init(&driver_os_dev, &driver_os_ops);
	driver_os_dev.owner = THIS_MODULE;

	if((ret = cdev_add(&driver_os_dev, devno, 1)) < 0){
		printk(KERN_ERR "cdev_add returned %d\n", ret);
		goto cdev_add_failed;
	}

	if((wq = create_workqueue("driver_os_slave_wq")) == NULL){
		printk(KERN_ERR "create_workqueue returned NULL\n");
		ret = -ENOMEM;
		goto create_workqueue_failed;
	}

	printk(KERN_INFO "driver_os initialized!\n");

	return 0;

create_workqueue_failed :
	cdev_del(&driver_os_dev);
cdev_add_failed:
	device_destroy(driver_os_cl, devno);
device_create_failed:
	class_destroy(driver_os_cl);
class_create_failed:
	unregister_chrdev_region(devno, 1);

	return ret;
}

static void __exit exiting(void)
{
	if(ssock != NULL){	// FIXME: race condition!?
		ssock->ops->shutdown(ssock, SHUT_RDWR);
	}

	if(wq != NULL)	destroy_workqueue(wq);

	cdev_del(&driver_os_dev);
	device_destroy(driver_os_cl, devno);
	class_destroy(driver_os_cl);
	unregister_chrdev_region(devno, 1);

	printk(KERN_INFO "driver_os removed!\n");
}

module_init(initialize);
module_exit(exiting);

static ssize_t driver_os_recv(struct socket *csock, char *buf, size_t size)
{
	struct msghdr msg;
	struct iovec iov;
	mm_segment_t oldfs;
	int ret;

	iov.iov_base = (void *)buf;
	iov.iov_len = (__kernel_size_t)size;

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;

	oldfs = get_fs();
	set_fs(KERNEL_DS);
	ret = sock_recvmsg(csock, &msg, size, 0);
	set_fs(oldfs);

	return ret;
}

// static ssize_t driver_os_send(struct socket *csock, char *buf, size_t size)
// {
// 	struct msghdr msg;
// 	struct iovec iov;
// 	mm_segment_t old_fs;
// 	int ret;
// 
// 	iov.iov_base = (void *)buf;
// 	iov.iov_len = (__kernel_size_t)size;
// 
// 	msg.msg_name = NULL;
// 	msg.msg_namelen = 0;
// 	msg.msg_iov = &iov;
// 	msg.msg_iovlen = 1;
// 	msg.msg_control = NULL;
// 	msg.msg_controllen = 0;
// 	msg.msg_flags = 0;
// 
// 	old_fs = get_fs();
// 	set_fs(KERNEL_DS);
// 	ret = sock_sendmsg(csock, &msg, size);
// 	set_fs(old_fs);
// 	
// 	return ret;
// }


struct socket *csock;
static void driver_os_slave_work_handler(struct work_struct *work) {
	int ret;

	printk (KERN_INFO "handler start\n");

	while (1) {
		ret = driver_os_recv(csock, sockbuf, 4096);
		
		if(ret > 0){
			sockbuf[ret] = 0;
			printk(KERN_INFO "recv: %s", sockbuf);
			datalen = ret;
			wake_up_interruptible(&driver_os_wait);
		}
		else {
			datalen = 0;
			wake_up_interruptible(&driver_os_wait);
			break;
		}
	}

	csock->ops->shutdown(csock, SHUT_RDWR);
	csock->ops->release(csock);
	sock_release(csock);

	printk (KERN_INFO "handler finishes\n");
}

DECLARE_WORK(driver_os_slave_work, driver_os_slave_work_handler);

static long my_ioctl(struct file *file,unsigned int ioctl_num, unsigned long ioctl_param)
{
	long ret = -EINVAL;
	char ip[16];
	struct sockaddr_in dest;

	printk (KERN_INFO "ioctl called\n");

	switch(ioctl_num){
		case 0 :
			copy_from_user (ip, (void*) ioctl_param, 16);

			csock = NULL;
			if((ret = sock_create_kern(AF_INET, SOCK_STREAM, IPPROTO_TCP, &csock)) < 0){
				printk(KERN_ERR "sock_create_lite returned %ld\n", ret);
				goto socket_create_failed;
			}

			memset (&dest, 0, sizeof (struct sockaddr_in));
			dest.sin_family = AF_INET;
			dest.sin_port = htons (8888);
			dest.sin_addr.s_addr = in_aton (ip);

			printk (KERN_INFO "ip = %s\n", ip);

			if ((ret = csock->ops->connect (csock, (struct sockaddr*) &dest, sizeof (struct sockaddr_in), !O_NONBLOCK)) < 0) {
				printk (KERN_ERR "socket connect failed, return %ld\n", ret);
				goto socket_connect_failed;
			}

			queue_work(wq, &driver_os_slave_work);

			break;
		default:
			break;
	}

socket_create_failed :
socket_connect_failed :
	return ret;
}

static int my_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int my_close(struct inode *inode, struct file *file)
{
	return 0;
}

static loff_t my_llseek(struct file *filp, loff_t off, int whence)
{
	return -1;
}

static ssize_t my_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
	int rlen;

	if(wait_event_interruptible(driver_os_wait, datalen > 0) != 0)	return -ERESTARTSYS;

	rlen = (count < datalen)?count:datalen;

	if(copy_to_user(buff, sockbuf, rlen))	return -EFAULT;

	return rlen;
}

static ssize_t my_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp)
{
	return -1;
}


// static *void my_mmap(struct file * filp, struct vm_area_struct *vma)
// {
// 	if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
// 				vma->vm_end - vma->vm_start,
// 				vma->vm_page_prot))
// 		return -EAGAIN;
// 	vma->vm_private_data = filp->private_data;
// 	vma->vm_op = &simple_remap_vm_ops;
// 
// }