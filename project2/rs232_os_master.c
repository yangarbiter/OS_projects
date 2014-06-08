#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/workqueue.h>
#include <linux/init.h>
#include <linux/mm.h>
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
/* static ssize_t driver_os_recv(struct socket *csock, char *buf, size_t size); */
static ssize_t driver_os_send(struct socket *csock, char *buf, size_t size);
/* static void driver_os_work_handler(struct work_struct *work); */
static int my_mmap(struct file *file, struct vm_area_struct *vma);

static struct file_operations driver_os_ops = {
	.owner = THIS_MODULE,
	.llseek = my_llseek,
	.read = my_read,
	.write = my_write,
	.unlocked_ioctl = my_ioctl,
	.open = my_open,
	.release = my_close,
	.mmap = my_mmap,
};

static dev_t devno;
static struct class *driver_os_cl;
static struct cdev driver_os_dev;
//static struct workqueue_struct *wq;
/* DECLARE_WORK(driver_os_work, driver_os_work_handler); */
/* DECLARE_WAIT_QUEUE_HEAD(driver_os_wait); */
static struct socket *ssock;
static char * sockbuf;
volatile int flag; /* 1 for unread */
/* static int datalen; */
struct socket *csock = NULL;

static int __init initialize(void)
{
	int ret;

	// KERN_ERR => <3> 代表發生錯誤
	if((ret = alloc_chrdev_region(&devno, 0, 1, "driver_os_master")) < 0){
		printk(KERN_ERR "alloc_chrdev_region returned %d\n", ret);
		return ret;
	}

	if((driver_os_cl = class_create(THIS_MODULE, "chardrv")) == NULL){
		printk(KERN_ERR "class_create returned NULL\n");
		ret = -ENOMEM;
		goto class_create_failed;
	}

	// 以driver_os_cl這個類創建/dev/driver_os檔案
	if(device_create(driver_os_cl, NULL, devno, NULL, "driver_os_master") == NULL){
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

	/* if((wq = create_workqueue("driver_os_wq")) == NULL){ */
	/* 	printk(KERN_ERR "create_workqueue returned NULL\n"); */
	/* 	ret = -ENOMEM; */
	/* 	goto create_workqueue_failed; */
	/* } */

	/* queue_work(wq, &driver_os_work); */

	printk(KERN_INFO "driver_os initialized!\n");


	return 0;

/* create_workqueue_failed: */
/* 	cdev_del(&driver_os_dev); */
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

	/* if(wq != NULL)	destroy_workqueue(wq); */

	cdev_del(&driver_os_dev);
	device_destroy(driver_os_cl, devno);
	class_destroy(driver_os_cl);
	unregister_chrdev_region(devno, 1);

	printk(KERN_INFO "driver_os removed!\n");
}

module_init(initialize);
module_exit(exiting);

/* static ssize_t driver_os_recv(struct socket *csock, char *buf, size_t size) */
/* { */
/* 	struct msghdr msg; */
/* 	struct iovec iov; */
/* 	mm_segment_t oldfs; */
/* 	int ret; */
/*  */
/* 	iov.iov_base = (void *)buf; */
/* 	iov.iov_len = (__kernel_size_t)size; */
/*  */
/* 	msg.msg_name = NULL; */
/* 	msg.msg_namelen = 0; */
/* 	msg.msg_iov = &iov; */
/* 	msg.msg_iovlen = 1; */
/* 	msg.msg_control = NULL; */
/* 	msg.msg_controllen = 0; */
/* 	msg.msg_flags = 0; */
/*  */
/* 	oldfs = get_fs(); */
/* 	set_fs(KERNEL_DS); */
/* 	ret = sock_recvmsg(csock, &msg, size, 0); */
/* 	set_fs(oldfs); */
/*  */
/* 	return ret; */
/* } */

static ssize_t driver_os_send(struct socket *csock, char *buf, size_t size)
{
	struct msghdr msg;
	struct iovec iov;
	mm_segment_t old_fs;
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

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	ret = sock_sendmsg(csock, &msg, size);
	set_fs(old_fs);

	return ret;
}

/*
static void driver_os_work_handler(struct work_struct *work)
{
	printk("handler start");
	int ret;
	struct sockaddr_in saddr;
	struct socket *csock = NULL;

	if(ssock != NULL){
		printk(KERN_ERR "ssock != NULL\n");
		return;
	}

	if((ret = sock_create_kern(AF_INET, SOCK_STREAM, IPPROTO_TCP, &ssock)) < 0){
		printk(KERN_ERR "sock_create returned %d\n", ret);
		ssock = NULL;
		return;
	}

	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(8881);
	saddr.sin_addr.s_addr = INADDR_ANY;

	if((ret = ssock->ops->bind(ssock, (struct sockaddr*)&saddr, sizeof(saddr))) < 0){
		printk(KERN_ERR "bind returned %d\n", ret);
		sock_release(ssock);
		ssock = NULL;
		return;
	}

	if((ret = ssock->ops->listen(ssock, 1)) < 0){
		printk(KERN_ERR "listen returned %d\n", ret);
		ssock->ops->release(ssock);
		sock_release(ssock);
		ssock = NULL;
		return;
	}

	while(1){
		if((ret = sock_create_kern(AF_INET, SOCK_STREAM, IPPROTO_TCP, &csock)) < 0){
			printk(KERN_ERR "sock_create_lite returned %d\n", ret);
			break;
		}

		if((ssock->ops->accept(ssock, csock, 0)) < 0){
			printk(KERN_ERR "accept returned %d\n", ret);
			sock_release(csock);
			break;
		}

		// got a connection, csock是要傳出去的socket

		while (1) {

			ret = driver_os_send(csock, sockbuf, 4096);

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
	}

	ssock->ops->release(ssock);
	sock_release(ssock);
	ssock = NULL;
}
*/
static long my_ioctl(struct file *file,unsigned int ioctl_num, unsigned long ioctl_param)
{
	// master 無需ioctl


	// 0 為
	int ret;
	if (ioctl_num == 0) {
		struct sockaddr_in saddr;

		if(ssock != NULL){
			printk(KERN_ERR "ssock != NULL\n");
			return 2;
		}

		if((ret = sock_create_kern(AF_INET, SOCK_STREAM, IPPROTO_TCP, &ssock)) < 0){
			printk(KERN_ERR "sock_create returned %d\n", ret);
			ssock = NULL;
			return 3;
		}
		printk("socket create\n");

		memset(&saddr, 0, sizeof(saddr));
		saddr.sin_family = AF_INET;
		saddr.sin_port = htons(8888);
		saddr.sin_addr.s_addr = INADDR_ANY;

		if((ret = ssock->ops->bind(ssock, (struct sockaddr*)&saddr, sizeof(saddr))) < 0){
			printk(KERN_ERR "bind returned %d\n", ret);
			sock_release(ssock);
			ssock = NULL;
			return 4;
		}
		printk("socket bind\n");

		if((ret = ssock->ops->listen(ssock, 1)) < 0){
			printk(KERN_ERR "listen returned %d\n", ret);
			ssock->ops->release(ssock);
			sock_release(ssock);
			ssock = NULL;
			return 5;
		}
		printk("socket listen\n");

		if((ret = sock_create_kern(AF_INET, SOCK_STREAM, IPPROTO_TCP, &csock)) < 0){
			printk(KERN_ERR "sock_create_lite returned %d\n", ret);
			return 6;
		}

		if((ssock->ops->accept(ssock, csock, 0)) < 0){
			printk(KERN_ERR "accept returned %d\n", ret);
			sock_release(csock);
			return 7;
		}
		printk("accepted\n");
	} else if(ioctl_num == 1) {
		int rlen;
		rlen = driver_os_send(csock, file->private_data, 4096);
		printk("sending data\n");
	}

	return 0;
}

static int my_open(struct inode *inode, struct file *file)
{
	csock = NULL;
	sockbuf = (char *)get_zeroed_page(GFP_KERNEL);
	file->private_data = sockbuf;
	return 0;
}

static int my_close(struct inode *inode, struct file *file)
{
	if(ssock != NULL){
		ssock->ops->release(ssock);
		sock_release(ssock);
		/* ssock->ops->shutdown(ssock, SHUT_RDWR); */
		ssock = NULL;
	}
	csock->ops->shutdown(csock, SHUT_RDWR);
	csock->ops->release(csock);
	sock_release(csock);
	free_page((unsigned long)sockbuf);
	file->private_data = NULL;
	csock = NULL;

	return 0;
}

static loff_t my_llseek(struct file *filp, loff_t off, int whence)
{
	return -1;
}

static ssize_t my_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
	return 0;
}

static ssize_t my_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp)
{
	int rlen;

	/* if(wait_event_interruptible(driver_os_wait, datalen >= 0) != 0)	return -ERESTARTSYS; */


	if(copy_from_user((void *)sockbuf, buff, count))	return -EFAULT;

	rlen = driver_os_send(csock, sockbuf, count);

	return rlen;
}

void vma_open(struct vm_area_struct *vma)
{
	printk("VMA open, virt %lx, phys %lx\n", vma->vm_start, vma->vm_pgoff << PAGE_SHIFT);
} 

static int mmap_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	struct page *page;
	char* buf;

	buf = (char *)vma->vm_private_data;
	page = virt_to_page(buf);
	get_page(page);
	vmf->page = page;
	return 0;
}

void vma_close(struct vm_area_struct *vma)
{
	printk("VMA close.\n");
}

static struct vm_operations_struct remap_vm_ops = {
	.open = vma_open,
	.close = vma_close, 
	.fault = mmap_fault,
};

static int my_mmap(struct file * file, struct vm_area_struct *vma)
{
	vma->vm_ops = &remap_vm_ops;
	vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP;
	vma->vm_private_data = file->private_data;
	vma_open(vma);
	return 0;
}
