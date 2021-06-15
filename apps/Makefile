CC = /home/bvlad/beaglebone_black/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc
LDFLAGS=-lm
#all: ioctl_test accel_read_cli print_to_hd44780
all: incline_show

app : ioctl_test.c
	$(CC) -o $@ $^
	
clean :
	rm ioctl_test accel_read_cli
	
deploy : ioctl_test accel_read_cli print_to_hd44780 incline_show
	scp $^ pi@192.168.7.4:
