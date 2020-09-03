/*
 * ledRGB_platform.c
 *
 *  Created on: Aug 19, 2020
 *      Author: bvlad
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>

struct led_dev{
	struct miscdevice led_misc_device;
	u32 led_mask;
	const char *led_name;
	char led_value[8];
};

#define BCM2710_PERI_BASE		0xfe000000
#define GPIO_BASE				(BCM2710_PERI_BASE + 0x200000)

#define GPIO_27		27
#define GPIO_22		22
#define GPIO_26		26

//set and clear each led
#define GPIO_27_INDEX	1 << (GPIO_27 % 32)
#define GPIO_22_INDEX	1 << (GPIO_22 % 32)
#define GPIO_26_INDEX	1 << (GPIO_26 % 32)

//select gpio output muxing
#define GPIO27_FUNC		1 << ((GPIO_27 % 10)*3)
#define GPIO26_FUNC		1 << ((GPIO_26 % 10)*3)
#define GPIO22_FUNC		1 << ((GPIO_22 % 10)*3)

//masks the GPIO functions
#define GPIO27_MASK		0b111 << ((GPIO_27 % 10)*3)
#define GPIO26_MASK		0b111 << ((GPIO_26 % 10)*3)
#define GPIO22_MASK		0b111 << ((GPIO_22 % 10)*3)

#define GPIO_SET_FUNCTION_LEDS	( GPIO27_FUNC | GPIO26_FUNC | GPIO22_FUNC )
#define GPIO_MASK_ALL_LEDS 		( GPIO27_MASK | GPIO26_MASK | GPIO22_MASK )
#define GPIO_SET_ALL_LEDS		( GPIO_27_INDEX | GPIO_22_INDEX | GPIO_26_INDEX )

#define GPFSEL2 		GPIO_BASE + 0x08
#define GPSET0			GPIO_BASE + 0x1C
#define GPCLR0			GPIO_BASE + 0x28

//declare __iomem pointers that will hold translated addresses
static void __iomem *GPFSEL2_V;
static void __iomem *GPSET0_V;
static void __iomem *GPCLR0_V;

static ssize_t led_write(struct file *file, const char __user *buff, size_t count, loff_t *ppos){
	const char *led_on="on";
	const char *led_off="off";
	struct led_dev *led_device;
	const char end_string='\0';

	pr_info("led_write() is called.\n");

	led_device=container_of(file->private_data, struct led_dev, led_misc_device);
	/*
	 * terminal echo adds \n char
	 * led_device->led_value="on\n" or "off\n" after copy_from_user;
	 */
	if(copy_from_user(led_device->led_value, buff, count)){
		pr_info("Bad copied value\n");
		return -EFAULT;
	}

	/*
	 * add \0 replacing \n in led_device->led_value in
	 * probe() initialisation
	 */


	//led_device->led_value[count-1]= (char) "\0";
	led_device->led_value[count-1]= end_string;

	pr_info("last char in led_value string is : %c", led_device->led_value[count-1]);

	pr_info("This message received from user space: %s", led_device->led_value);

	if(!strcmp(led_device->led_value, led_on))
		iowrite32(led_device->led_mask, GPSET0_V);
	else if(!strcmp(led_device->led_value, led_off))
		iowrite32(led_device->led_mask, GPCLR0_V);
	else{
		pr_info("Bad value\n");
		return -EINVAL;
	}

	pr_info("led_write() is exiting");
	return count;
}

static ssize_t led_read(struct file *file, char __user *buff, size_t count, loff_t *ppos){
	struct led_dev *led_device;

	pr_info("led_read() is called.\n");

	led_device = container_of(file->private_data, struct led_dev, led_misc_device);

	if(*ppos == 0){
		if(copy_to_user(buff, &led_device->led_value, sizeof(led_device->led_value))){
			pr_info("Failed to return led_value to user space\n");
			return -EFAULT;
		}
		*ppos+=1;
		return sizeof(led_device->led_value);
	}

	pr_info("led_read() exits\n");

	return 0;
}

static const struct file_operations led_fops = {
		.owner= THIS_MODULE,
		.read = led_read,
		.write = led_write,
};

static int __init led_probe(struct platform_device *pdev){
	struct led_dev *led_device;
	int ret_val;
	char led_val[8]="off\n";

	pr_info("leds_probe() enter\n");

	led_device=devm_kzalloc(&pdev->dev, sizeof(struct led_dev), GFP_KERNEL);

	of_property_read_string(pdev->dev.of_node, "label", &led_device->led_name);
	led_device->led_misc_device.minor=MISC_DYNAMIC_MINOR;
	led_device->led_misc_device.name=led_device->led_name;
	led_device->led_misc_device.fops=&led_fops;

	if(strcmp(led_device->led_name, "red")==0){
		led_device->led_mask = GPIO_27_INDEX;
	}
	else if(strcmp(led_device->led_name, "green")==0){
			led_device->led_mask = GPIO_22_INDEX;
		}
	else if(strcmp(led_device->led_name, "blue")==0){
				led_device->led_mask = GPIO_26_INDEX;
			}
	else{
		pr_info("bad device tree value.\n");
		return -EINVAL;
	}

	//Initialise the led status to off
	memcpy(led_device->led_value, led_val, sizeof(led_val));

	ret_val=misc_register(&led_device->led_misc_device);
	if(ret_val) return ret_val;

	platform_set_drvdata(pdev, led_device);

	pr_info("leds_probe exit");

	return 0;
}

static int __exit led_remove(struct platform_device *pdev){

	struct led_dev *led_device = platform_get_drvdata(pdev);

	pr_info("leds_remove enter.\n");
	misc_deregister(&led_device->led_misc_device);
	pr_info("leds_remove exit\n");
	return 0;
}

static const struct of_device_id my_of_ids[] = {
		{.compatible="arrow,RGBleds"},
		{},
};

MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver led_platform_driver = {
		.probe=led_probe,
		.remove=led_remove,
		.driver={
				.name="RGBleds",
				.of_match_table=my_of_ids,
				.owner=THIS_MODULE,
		}

};

static int led_init(void){
	int ret_val;
	u32 GPFSEL_read, GPFSEL_write;
	pr_info("demo_init neter.\n");

	ret_val=platform_driver_register(&led_platform_driver);
	if(ret_val!=0){
		pr_err("platform value returned %d.\n", ret_val);
		return ret_val;
	}

	GPFSEL2_V=ioremap(GPFSEL2, sizeof(u32));
	GPSET0_V=ioremap(GPSET0, sizeof(u32));
	GPCLR0_V=ioremap(GPCLR0, sizeof(u32));

	GPFSEL_read=ioread32(GPFSEL2_V); //read current value

	/*
	 * set to 0 3 bits of each FSEL and keep equal the rest of the bits
	 * then set to 1 the first bit of each FSEL (function) to set 3 GPIOs to output
	 */

	GPFSEL_write=(GPFSEL_read & ~GPIO_MASK_ALL_LEDS) | (GPIO_SET_FUNCTION_LEDS & GPIO_MASK_ALL_LEDS);

	iowrite32(GPFSEL_write, GPFSEL2_V);
	iowrite32(GPIO_SET_ALL_LEDS, GPCLR0_V);

	pr_info("demo_init exit.\n");

	return 0;
}

static void led_exit(void){
	pr_info("led driver enter.\n");

	iowrite32(GPIO_SET_ALL_LEDS, GPCLR0_V); //clear all the leds

	iounmap(GPFSEL2_V);
	iounmap(GPSET0_V);
	iounmap(GPCLR0_V);

	platform_driver_unregister(&led_platform_driver);

	pr_info("led_driver exit \n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vlad Barbalata");
MODULE_DESCRIPTION("This is a platform driver that turns on and off 3 RGB leds");



