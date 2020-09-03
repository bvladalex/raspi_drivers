/*
 * ledRGB_class_platform.c
 *
 *  Created on: Sep 1, 2020
 *      Author: bvlad
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/uaccess.h>
#include <linux/leds.h>
#include <linux/delay.h>

#define GPIO_27		27
#define GPIO_22		22
#define GPIO_26		26

#define GPFSEL2_offset		0x08
#define GPSET0_offset		0x1C
#define GPCLR0_offset		0x28

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

struct led_dev{
	u32 led_mask;
	void __iomem *base;
	struct led_classdev cdev;
};

static void led_control(struct led_classdev *led_cdev, enum led_brightness b){
	struct led_dev *led = container_of(led_cdev,struct led_dev,cdev);
	iowrite32(GPIO_SET_ALL_LEDS,led->base+GPCLR0_offset);
	if(b!=LED_OFF)
		iowrite32(led->led_mask,led->base+GPSET0_offset);
	else
		iowrite32(led->led_mask,led->base+GPCLR0_offset);

}

static int __init ledclass_probe(struct platform_device *pdev){

	void __iomem *g_ioremap_addr;
	struct device_node *child;
	struct resource *r;
	u32 GPFSEL_read, GPFSEL_write;
	struct device *dev=&pdev->dev;
	int ret=0;
	int count;

	pr_info("platform_probe enter.\n");

	/*get first memory resource from device tree*/
	r=platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(!r){
		pr_err("IORESOURCE_MEM, 0 does not exist\n");
		return -EINVAL;
	}
	pr_info("r->start = 0x%081x\n", (unsigned int)r->start);
	pr_info("r->end = 0x%081x\n", (unsigned int)r->end);

	/*ioremap our memory region*/
	g_ioremap_addr=devm_ioremap(dev, r->start, resource_size(r));
	if(!g_ioremap_addr){
		pr_err("ioremap failed \n");
		return -ENOMEM;
	}

	count =of_get_child_count(dev->of_node);
	if(!count)
		return -EINVAL;

	pr_info("these are %d nodes:\n", count);

	GPFSEL_read=ioread32(g_ioremap_addr + GPFSEL2_offset);  //read actual value

	GPFSEL_write = (GPFSEL_read & ~ GPIO_MASK_ALL_LEDS)|(GPIO_SET_FUNCTION_LEDS & GPIO_MASK_ALL_LEDS);

	iowrite32(GPFSEL_write, g_ioremap_addr + GPFSEL2_offset);  //set dir leds to output
	iowrite32(GPIO_SET_ALL_LEDS, g_ioremap_addr + GPCLR0_offset); //clear all leds, output is low

	for_each_child_of_node(dev->of_node, child){

		struct led_dev *led_device;
		struct led_classdev *cdev;
		led_device=devm_kzalloc(dev, sizeof(*led_device), GFP_KERNEL);
		if(!led_device)
			return -ENOMEM;

		cdev=&led_device->cdev;

		led_device->base=g_ioremap_addr;

		of_property_read_string(child, "label", &cdev->name);

		if(strcmp(cdev->name, "red")==0){
			led_device->led_mask = GPIO_27_INDEX;
		}
		else if(strcmp(cdev->name, "green")==0){
			led_device->led_mask = GPIO_22_INDEX;
		}
		else if(strcmp(cdev->name, "blue")==0){
			led_device->led_mask = GPIO_26_INDEX;
		}
		else{
			pr_info("bad device tree value.\n");
			return -EINVAL;
		}

		//Disable timer trigger until led is on
		led_device->cdev.brightness=LED_OFF;
		led_device->cdev.brightness_set=led_control;

		ret=devm_led_classdev_register(dev, &led_device->cdev);
		if(ret){
			dev_err(dev, "failed to register the led %s\n", cdev->name);
			of_node_put(child);
			return ret;
		}
	}

	pr_info("leds_probe exit\n");

	return 0;
}

static int __exit ledclass_remove(struct platform_device *pdev){
	pr_info("ledclass_remove() enters");
	pr_info("ledclass_remove() exits");

	return 0;
}

static const struct of_device_id my_of_ids[]={
	{.compatible="arrow,RGBclassleds"},
	{},
};

MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver led_platform_driver = {
	.probe=ledclass_probe,
	.remove=ledclass_remove,
	.driver={
			.name="RGBclassleds",
			.of_match_table=my_of_ids,
			.owner=THIS_MODULE,
	}
};

static int ledRGBclass_init(void){
	int ret_val;
	pr_info("demo_init enters");
	ret_val=platform_driver_register(&led_platform_driver);
	if(ret_val!=0){
		pr_err("platform value returned %d\n", ret_val);
		return ret_val;
	}

	pr_info("demo_init exit\n");
	return 0;
}

static void ledRGBclass_exit(void){
	pr_info("led driver enter \n");
	platform_driver_unregister(&led_platform_driver);
	pr_info("led_driver exit \n");
}

module_init(ledRGBclass_init);
module_exit(ledRGBclass_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vlad Barbalata");
MODULE_DESCRIPTION("This is a platform driver that turns on and off 3 RGB leds using the LED subsystem");

