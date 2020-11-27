/*
 * led_timer.c
 *
 *  Created on: Nov 25, 2020
 *      Author: bvlad
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/timer.h>
#include <linux/device.h>

#define BCM2710_PERI_BASE 	0x3F000000
#define GPIO_BASE			BCM2710_PERI_BASE + 0x200000

struct GpioRegisters
{
	uint32_t GFPSEL[6];
	uint32_t Reserved1;
	uint32_t GPSET[2];
	uint32_t Reserved2;
	uint32_t GPCLR[2];
};

static struct GpioRegisters *s_pGpioRegisters;

static void SetGPIOFunction(int GPIO, int functionCode){
	int registerIndex = GPIO/10;
	int bit = (GPIO%10)*3;

	unsigned oldValue = s_pGpioRegisters->GFPSEL[registerIndex];
	unsigned mask=0b111<<bit;
	pr_info("Changing function of GPIO%d from %x to %x\n", GPIO, (oldValue>>bit)&0b111,functionCode);
	s_pGpioRegisters->GFPSEL[registerIndex]=(oldValue&~mask)|((functionCode<<bit)&mask);
}

static void SetGPIOOutputValue(int GPIO, bool outputValue){
	if(outputValue)
		s_pGpioRegisters->GPSET[GPIO/32]=(1<<(GPIO%32));
	else
		s_pGpioRegisters->GPCLR[GPIO/32]=(1<<(GPIO%32));
}

static struct timer_list s_BlinkTimer;
static int s_BlinkPeriod = 1000;
static const int LedGpioPin = 27;


//static void BlinkTimerHandler(unsigned long used){
static void BlinkTimerHandler(struct timer_list* used){
	static bool on = false;
	on = !on;
	SetGPIOOutputValue(LedGpioPin, on);
	mod_timer(&s_BlinkTimer, jiffies+msecs_to_jiffies(s_BlinkPeriod));
}

static ssize_t set_period(struct device *dev, struct device_attribute* attr, const char* buf, size_t count){
	long period_value = 0;
	if(kstrtol(buf, 10, &period_value)<0)
		return -EINVAL;
	if(period_value<10)
		return -EINVAL;

	s_BlinkPeriod = period_value;
	return count;
}

static DEVICE_ATTR(period, S_IWUSR, NULL, set_period);

static struct miscdevice led_miscdevice = {
		.minor = MISC_DYNAMIC_MINOR,
		.name = "ledred",
};

static int __init my_probe(struct platform_device *pdev){

	int result, ret_val;
	struct device *dev = &pdev->dev;
	dev_info(dev,"platform_probe enter \n");
	s_pGpioRegisters=(struct GpioRegisters *)devm_ioremap(dev,GPIO_BASE, sizeof(struct GpioRegisters));
	SetGPIOFunction(LedGpioPin, 0b001); //configure pin as output

	timer_setup(&s_BlinkTimer, BlinkTimerHandler, 0);

	result = mod_timer(&s_BlinkTimer, jiffies + msecs_to_jiffies(s_BlinkPeriod));

	ret_val=device_create_file(&pdev->dev, &dev_attr_period);

	if(ret_val!=0){
		dev_err(dev, "failed to create sysfs entry");
		return ret_val;
	}

	ret_val=misc_register(&led_miscdevice);

	if(ret_val!=0){
		dev_err(dev, "could not register the misc device mydev");
		return ret_val;
	}

	dev_info(dev, "mydev: got minor %i\n", led_miscdevice.minor);
	dev_info(dev, "platform_probe exit \n");

	return 0;
}

static int __exit my_remove(struct platform_device *pdev){
	dev_info(&pdev->dev, "platform_remove enter\n");
	misc_deregister(&led_miscdevice);
	device_remove_file(&pdev->dev, &dev_attr_period);
	SetGPIOFunction(LedGpioPin, 0);
	del_timer(&s_BlinkTimer);
	dev_info(&pdev->dev, "platform_remove exit\n");
	return 0;
}

static const struct of_device_id my_of_ids[] = {
		{.compatible = "arrow,ledred"},
		{},
};

static struct platform_driver my_platform_driver = {
		.probe=my_probe,
		.remove=my_remove,
		.driver={
				.name="ledred",
				.of_match_table=my_of_ids,
				.owner=THIS_MODULE,
		}
};

module_platform_driver(my_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vlad Barbalata");
MODULE_DESCRIPTION("This is a button INT platform driver");
