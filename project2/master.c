#include<stdio.h>
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

	if ((f_fd = open(argv[1], O_RDONLY)) < 0) {
		perror("file open error");
	}
	if ((dev_fd = open(DEV_PATH, O_WRONLY | O_NONBLOCK)) < 0) {
		perror("device open error");
	}
	
	/* ioctl(dev_fd, 1, NULL);  //master */

	if(strcmp(argv[2], "fcntl") == 0){
		int ret;
		if ((ret = ioctl(dev_fd, 0, NULL)) != 0) {
			fprintf(stderr, "ioctl fail, return %d\n", ret);
			perror("");
		}
		fprintf(stderr, "ioctl success\n");


		int s;
		char buf[512];

		while((s = read(f_fd, buf, 512)) != 0){
			if(s == -1){
			}
			while(s != 0){
				int ret = write(dev_fd, buf, s);
				if(ret == -1){
				}
				s -= ret;
			}
		}
	}else if(strcmp(argv[2], "mmap") == 0){
		int f_size, mmap_size, page_size = sysconf(_SC_PAGE_SIZE);//, f_offset = 0; 
		void *f_map, *dev_map;

		lseek(f_fd, 0, SEEK_END);
		f_size = lseek(f_fd, 0, SEEK_CUR);
		lseek(f_fd, 0, SEEK_SET);

		mmap_size = mmap_size*((f_size/page_size)+1);
		
		f_map = mmap(NULL, mmap_size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, f_fd, 0);
		dev_map = mmap(NULL, mmap_size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, dev_fd, 0);

		memcpy(dev_map, f_map, mmap_size);

		munmap(f_map, mmap_size);
		munmap(dev_map, mmap_size);
	}
	close(f_fd);
	close(dev_fd);

	return 0;
	
}

