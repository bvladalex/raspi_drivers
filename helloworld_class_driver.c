/*
 * helloworld_class_driver.c
 *
 *  Created on: Aug 9, 2020
 *      Author: bvlad
 */
#include  <linux/module.h>

/*header files for character devices */

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>

#define DEVICE_NAME "mydev"
#define CLASS_NAME "hello_class"

static struct cdev my_dev;
static struct class *helloClass;
dev_t dev;

static int my_dev_open(struct inode *inode, struct file *file){
	pr_info("my_dev_open() is called.\n");
	return 0;
}

static int my_dev_close(struct inode *inode, struct file *file){
	pr_info("my_dev_close() is called.\n");
	return 0;
}

static long my_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	pr_info("my_dev_ioctl is called. cmd = %d, arg = %ld\n", cmd, arg);
	return 0;
}

//declare a file_operations structure

static const struct file_operations my_dev_fops = {
		.owner = THIS_MODULE,
		.open = my_dev_open,
		.release = my_dev_close,
		.unlocked_ioctl = my_dev_ioctl,
};

static int __init hello_init(void){
	int ret;
	dev_t dev_no;
	int Major;
	struct device *helloDevice;

	pr_info("Hello world init\n");

	//Allocate device numbers
	ret=alloc_chrdev_region(&dev_no,0,1, DEVICE_NAME);
	if(ret<0){
		pr_info("Unable to allocate major number \n");
		return ret;
	}

	// Get the device identifiers
	Major= MAJOR(dev_no);
	dev=MKDEV(Major,0);

	pr_info("Allocated corectly with major number %d\n", Major);

	// Initialise the cdev structure and add it to kernel space
	cdev_init(&my_dev, &my_dev_fops);
	ret=cdev_add(&my_dev, dev, 1);
	if(ret<0){
		unregister_chrdev_region(dev,1);
		pr_info("Unable to add cdev\n");
		return ret;
	}

	////Register device class

	helloClass=class_create(THIS_MODULE, CLASS_NAME);
	if(IS_ERR(helloClass)){
		unregister_chrdev_region(dev,1);
		cdev_del(&my_dev);
		pr_info("Failed to register device class\n");
		return PTR_ERR(helloClass);
	}
	pr_info("device class registered correctly\n");

	///Create device node named DEVICE_NAME

	helloDevice = device_create(helloClass, NULL, dev, NULL, DEVICE_NAME);
	if(IS_ERR(helloDevice)){
		class_destroy(helloClass);
		cdev_del(&my_dev);
		unregister_chrdev_region(dev,1);
		pr_info("Failed to create the device\n");
		return PTR_ERR(helloDevice);
	}
	pr_info("The device is registered correctly");


	return 0;
}

static void __exit hello_exit(void){
	device_destroy(helloClass, dev);
	class_destroy(helloClass);
	cdev_del(&my_dev);
	unregister_chrdev_region(dev,1);
	pr_info("Hello world exit\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vlad Barbalata");
MODULE_DESCRIPTION("This is the module that interacts with the ioctl system call");

