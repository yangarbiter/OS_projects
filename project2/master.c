#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>

const char* DEV_PATH = "/dev/rs232_os"

int main(int argc, char argv[])
{
	int dev_fd, f_fd;
	if(argc != 3){
		return 1;
	}

	dev_fd = open(DEV_PATH, O_WRONLY);
	open(argv[1], O_RDONLY);

	if(strcmp(argv[2], "fcntl") == 0){
		int s;
		char buf[512];

		while(s = read(f_fd, buf, 512) != 0){
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
		/*int f_size, mmap_size = sysconf(_SC_PAGE_SIZE), f_offset = 0; 
		char* buf;

		lseek(fp, 0, SEEK_END);
		f_size = lseek(fd, 0, SEEK_CUR);
		buf = (char*)malloc(sizeof(char) * f_size);

		mmap_size = mmap_size*((f_size/page_size)+1);
		
		buf = mmap(NULL, mmap_size, PROT_READ|PROT_READ, MAP_FILE|MAP_SHARED, f_fd, 0);

		while(f_offset < f_size){
			int ret = write(f_fd, buf+f_offset, f_size-f_offset);
			if(ret == -1){
			}
			f_offset += ret;
		}

		munmap(buf, mmap_size);*/
	}
	fclose(f_fd);
	close(dev_fd);

	return 0;
	
}

