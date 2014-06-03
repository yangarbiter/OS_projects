obj-m += rs232_os_slave.o rs232_os_master.o
CFLAGS_rs232_os_slave := -Wall
CFLAGS_rs232_os_master := -Wall

all: module
	gcc master.c -o master
	gcc slave.c -o slave

module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
