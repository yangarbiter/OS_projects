#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<sys/ioctl.h>

const char* DEV_PATH = "/dev/rs232_os_slave";

int main(int argc, char* argv[])
{
	int dev_fd;

	if(argc != 3){
		return 1;
	}

	dev_fd = open(DEV_PATH, O_RDONLY);

	ioctl(dev_fd, 0, argv[1]);  //slave pass ip and bulid connection

	if(strcmp(argv[2], "fcntl") == 0){
		int s, f_fd = open(argv[1], O_WRONLY);
		char buf[512];

		while(s = read(dev_fd, buf, 512) != 0){
			if(s == -1){
			}
			while(s != 0){
				int ret = write(f_fd, buf, s);
				if(ret == -1){
				}
				s -= ret;
			}
		}
		close(f_fd);
	}else if(strcmp(argv[2], "mmap") == 0){

	}

	close(dev_fd);

	return 0;
	
}

