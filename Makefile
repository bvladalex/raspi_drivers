#obj-m += helloworld.o helloworld_char_drv.o helloworld_class_driver.o hellokeys.o ledRGB_platform.o
#obj-m += ledRGB_platform.o ledRGB_class_platform.o
#obj-m += tc74_temp_sensor.o int_key.o led_timer.o int_key_wait.o keyled_class.o
obj-m += linked_list.o i2c_rpi_accel.o mcp4151.o hd44780-i2c.o

KERNEL_DIR ?= $(HOME)/raspi/linux

hd44780-i2c-objs:= hd44780-dev.o

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