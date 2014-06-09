#include<stdio.h>
#include <sys/stat.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<sys/mman.h>

const char* DEV_PATH = "/dev/driver_os_master";

int main(int argc, char* argv[])
{
	int dev_fd, f_fd;
	if(argc != 3){
		return 1;
	}

	if ((f_fd = open(argv[1], O_RDWR)) < 0) {
		perror("file open error");
	}
	if ((dev_fd = open(DEV_PATH, O_RDWR | O_NONBLOCK)) < 0) {
		perror("device open error");
	}
	int ret;
	if ((ret = ioctl(dev_fd, 0, NULL)) != 0) {
		fprintf(stderr, "ioctl 0 fail, return %d\n", ret);
		return -1;
	}

	struct stat st;
	fstat(f_fd, &st);
	fprintf(stderr, "file size = %u\n", (unsigned int) st.st_size);
	if ((ret = ioctl(dev_fd, 1, st.st_size)) != 0) {
		fprintf(stderr, "ioctl 1 fail, return %d\n", ret);
		return -1;
	}

	fprintf(stderr, "前置完成\n");

	if(strcmp(argv[2], "fcntl") == 0) {
		int s;
		char buf[512];

		while((s = read(f_fd, buf, 512)) != 0){
			if(s == -1){
				perror ("read == -1");
				break;
			}
			while(s != 0){
				int ret = write(dev_fd, buf, s);
				if(ret == -1){
					perror ("ret == -1");
					return 1;
				}
				s -= ret;
			}
		}
	} else if(strcmp(argv[2], "mmap") == 0){
		int f_size, mmap_size, page_size = sysconf(_SC_PAGE_SIZE);
		void *f_map, *dev_map;

		lseek(f_fd, 0, SEEK_END);
		f_size = lseek(f_fd, 0, SEEK_CUR);
		lseek(f_fd, 0, SEEK_SET);

		mmap_size = page_size*((f_size/page_size)+1);

		f_map = mmap(NULL, mmap_size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, f_fd, 0);
		if(f_map == MAP_FAILED){
			perror ("dev mmap failed.");
			return 1;
		}
		dev_map = mmap(NULL, mmap_size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, dev_fd, 0);
		if(dev_map == MAP_FAILED){
			perror ("dev mmap failed.");
			return 1;
		}

		memcpy(dev_map, f_map, mmap_size);

		ioctl(dev_fd, 1, NULL);  //send data

		munmap(f_map, mmap_size);
		munmap(dev_map, mmap_size);
	}
	close(f_fd);
	if (close(dev_fd) != 0) {
		perror("close device error");
	}

	return 0;

}

