#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<sys/mman.h>

const char* DEV_PATH = "/dev/driver_os_slave";

int main(int argc, char* argv[])
{
	int dev_fd, f_fd;

	if(argc != 4){
		fprintf (stderr, "Wrong arguments\n");
		return 1;
	}

	f_fd = open(argv[1], O_WRONLY);
	dev_fd = open(DEV_PATH, O_RDONLY);

	ioctl(dev_fd, 0, argv[3]);  //slave pass ip and bulid connection

	if(strcmp(argv[2], "fcntl") == 0){
		int s;
		char buf[512];

		while((s = read(dev_fd, buf, 512)) != 0){
			if(s == -1){
				perror ("");
				break;
			}
			while(s != 0){
				int ret = write(f_fd, buf, s);
				if(ret == -1){
				}
				s -= ret;
			}
		}
	}else if(strcmp(argv[2], "mmap") == 0){
		int f_size, mmap_size, page_size = sysconf(_SC_PAGE_SIZE), f_offset = 0; 
		void *f_map, *dev_map;

		lseek(f_fd, 0, SEEK_END);
		f_size = lseek(f_fd, 0, SEEK_CUR);
		lseek(f_fd, 0, SEEK_SET);

		mmap_size = mmap_size*((f_size/page_size)+1);
		
		f_map = mmap(NULL, mmap_size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, f_fd, 0);
		dev_map = mmap(NULL, mmap_size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, dev_fd, 0);

		memcpy(f_map, dev_map, mmap_size);

		munmap(f_map, mmap_size);
		munmap(dev_map, mmap_size);

	}

	close(f_fd);
	close(dev_fd);

	return 0;
	
}

