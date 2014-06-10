[http://rswiki.csie.org/dokuwiki/courses:102_2:project_2](project detail)

#### Setup
```
git clone https://github.com/yangarbiter/OS_projects
cd project2
make
sudo insmod rs232_os_master.ko
sudo insmod rs232_os_slave.ko
```

#### Usage:
  ./master F M IP
  
  ./slave F M
  
  * F: file to read/write to.
  * M: fcntl/mmap/devmmap
    * fcntl: use read/write to connect to device and file.
    * mmap: use read/write to connect to device and use mmap to connect to file.
    * devmmap: use mmap to connect to device and file.
  * IP: ip address of the master device.


#### ioctl spec
* rs232_os_slave: ioctl
  * 0: ioctl_param → porinter to ip address. Connect socket to specified ip address on port 8888.
  * 1: receive all data.
  * 3: ioctl_param → porinter to file size(int). Get file size.
  * 4: Receive data with return value of how many bytes are sended/received.

* rs232_os_master: ioctl
  * 0: Create a scoket on port 8888.
  * 1: receive all data.
  * 3: ioctl_param → (int) size. Send the specified number of bytes(ioctl_param) in struct file -> private_data  through socket.
