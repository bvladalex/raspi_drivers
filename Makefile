#obj-m += helloworld.o helloworld_char_drv.o helloworld_class_driver.o hellokeys.o ledRGB_platform.o
#obj-m += ledRGB_platform.o ledRGB_class_platform.o
obj-m += tc74_temp_sensor.o int_key.o

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