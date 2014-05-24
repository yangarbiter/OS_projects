#include<stdio.h>
#include<string.h>

const char* DEV_PATH = "/dev/rs232

int main(int argc, char argv[])
{
	if(argc != 3){
		return 1;
	}

	if(strcmp(argv[2], "fcntl") == 0){
		int s, f_fd = open(argv[1], "w"), dev_fd = open(DEV_PATH, "r");
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
		fclose(dev_fd);
		fclose(f_fd);
	}else if(strcmp(argv[2], "mmap") == 0){
	}

	return 0;
	
}

