MODULENAME = rs232_os

obj-m += $(MODULENAME).o
CFLAGS_$(MODULENAME) := -Wall

all: module
	gcc master.c -o master
	gcc slave.c -o slave

module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
