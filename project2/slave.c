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

	if ((f_fd = open(argv[1], O_RDWR | O_CREAT, 0644)) < 0) {
		perror("file open error");
	}
	if ((dev_fd = open(DEV_PATH, O_RDWR)) < 0) {
		perror("device open error");
	}

	ioctl(dev_fd, 0, argv[3]);  //slave pass ip and bulid connection

	if(strcmp(argv[2], "fcntl") == 0){
		int s;
		char buf[512];

		while((s = read(dev_fd, buf, 512)) != 0){
			if(s == -1){
				perror ("read == -1");
				break;
			}
			while(s != 0){
				int ret = write(f_fd, buf, s);
				if(ret == -1){
					perror ("ret == -1");
					return 1;
				}
				s -= ret;
			}
		}
	}else if (strcmp (argv[2], "mmap") == 0) {
		int f_size, mmap_size, page_size = sysconf(_SC_PAGE_SIZE), page_count; 
		char *writep;
		int readn, readn_total;
		void *f_map;

		/* get f_size */
		ioctl (dev_fd, 3, &f_size);

		ftruncate(f_fd, f_size);
		page_count = (f_size + page_size - 1) / page_size;
		mmap_size = page_size * page_count;
		
		f_map = mmap(NULL, mmap_size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, f_fd, 0);
		if(f_map == MAP_FAILED){
			perror ("dev mmap failed.");
			return 1;
		}

		writep = (char *)f_map;
		readn_total = 0;
		while (readn_total < f_size) {
			readn = read (dev_fd, writep + readn_total, 512);
			readn_total += readn;
		}

		ioctl (dev_fd, 6, page_count);
		ioctl (dev_fd, 5, f_map);

		munmap(f_map, mmap_size);
	}else if(strcmp(argv[2], "devmmap") == 0){
		int f_size, mmap_size, page_size = sysconf(_SC_PAGE_SIZE), page_count, received;
		void *f_map, *dev_map;

		/* get f_size */
		ioctl (dev_fd, 3, &f_size);

		ftruncate(f_fd, f_size);
		page_count = (f_size + page_size - 1) / page_size;
		mmap_size = page_size * page_count;
		
		f_map = mmap(NULL, mmap_size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, f_fd, 0);
		if(f_map == MAP_FAILED){
			perror ("dev mmap failed.");
			return 1;
		}
		dev_map = mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, dev_fd, 0);
		if(dev_map == MAP_FAILED){
			perror ("dev mmap failed.");
			return 1;
		}

		received = 0;
		while( f_size > received ){
			int receive;
			receive = ioctl(dev_fd, 4, NULL);  //receive data
			memcpy(f_map+received, dev_map, receive);
			received += receive;
		}

		munmap(f_map, mmap_size);
		munmap(dev_map, mmap_size);
	}

	close(f_fd);
	close(dev_fd);

	return 0;
	
}

