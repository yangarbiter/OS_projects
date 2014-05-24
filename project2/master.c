#include<stdio.h>
#include<string.h>

const char* DEV_PATH = "/dev/rs232

int main(int argc, char argv[])
{
	if(argc != 3){
		return 1;
	}

	if(strcmp(argv[2], "fcntl") == 0){
		int f_fd = open(argv[1], "r"), dev_fd = open(DEV_PATH, "w"), s;
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
		fclose(dev_fd);
		fclose(f_fd);
	}else if(strcmp(argv[2], "mmap") == 0){
		int f_size, mmap_size = sysconf(_SC_PAGE_SIZE), f_offset = 0; 
		char* buf;

		lseek(fp, 0, SEEK_END);
		f_size = lseek(fd, 0, SEEK_CUR);
		buf = (char*)malloc(sizeof(char) * f_size);

		mmap_size = mmap_size*((f_size/page_size)+1);
		
		buf = mmap(NULL, mmap_size, PROT_READ, 0, f_fd, 0);

		while(f_offset < f_size){
			int ret = write(f_fd, buf+f_offset, f_size-f_offset);
			if(ret == -1){
			}
			f_offset += ret;
		}

		munmap(buf, mmap_size);
	}

	return 0;
	
}

