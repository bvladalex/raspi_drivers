CC = /home/bvlad/beaglebone_black/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc

all: ioctl_test

app : ioctl_test.c
	$(CC) -o $@ $^
	
clean :
	rm_ioctl_test
	
deploy : ioctl_test
	scp $^ pi@192.168.7.4:
