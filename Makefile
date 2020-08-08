obj-m += helloworld.o

KERNEL_DIR ?= $(HOME)/raspi/linux

all:
	make -C $(KERNEL_DIR) \
			ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- \
			M=$(PWD) modules
			
clean:
	make -C $(KERNEL_DIR) \
			ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- \
			M=$(PWD) clean
			
deploy:
	scp *.ko pi@192.168.7.4:
	
#hello:
#	make -C $(KERNEL_DIR) \
#			ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- \
#			M=$(PWD)