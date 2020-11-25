/*
 * tc74_temp_sensor.c
 *
 *  Created on: Oct 27, 2020
 *      Author: bvlad
 */

#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/uaccess.h>

/*Private device structure*/
struct tc74_dev {
	struct i2c_client *client;
	struct miscdevice tc74_miscdevice;
	char name[8]; //tc74XX
};

//reading data from /dev/tc74XX

static ssize_t tc74_read_file(struct file *file, char __user *userbuf,size_t count, loff_t *ppos){
	int tempval, size;
	char buf[3];
	struct tc74_dev *tc74;
	const __u8 reg = 0x00;

	tc74= container_of(file->private_data, struct tc74_dev, tc74_miscdevice);

	//store IO expander input to tempval int variable

	//tempval = i2c_smbus_read_byte(tc74->client);
	//use specific reg read
	tempval = i2c_smbus_read_byte_data(tc74->client,reg);

	if (tempval < 0)
		return -EFAULT;

	size = sprintf(buf, "%02x", tempval);

	buf[size]= '\n';

	//send size+1 to include \n character
	if(*ppos == 0){
		if(copy_to_user(userbuf, buf, size+1)){
			pr_info("Failed to return led_value to user space");
			return -EFAULT;
		}
		*ppos+=1;
		return size+1;
	}
	return 0;
}
	static ssize_t tc74_write_file(struct file *file, const char __user *userbuf, size_t count, loff_t *ppos){
		int ret;
		unsigned long val;
		char buf[4];
		struct tc74_dev *tc74;

		tc74=container_of(file->private_data, struct tc74_dev, tc74_miscdevice);

		dev_info(&tc74->client->dev, "tc74_write_file entered on %s\n", tc74->name);

		dev_info(&tc74->client->dev, "we have written %zu characters\n", count);

		if(copy_from_user(buf, userbuf, count)){
			dev_err(&tc74->client->dev, "Bad copied value\n");
			return -EFAULT;
		}

		buf[count-1]='\0';

		//convert string to unsigned long
		ret=kstrtoul(buf, 0, &val);
		if(ret)
			return -EINVAL;

		dev_info(&tc74->client->dev, "the value is %lu\n", val);

		ret=i2c_smbus_write_byte(tc74->client, val);
		if(ret<0)
			dev_err(&tc74->client->dev, "the device is not found\n");

		dev_info(&tc74->client->dev, "tc74_write_file exited on %s\n", tc74->name);

		return count;
	}

	static const struct file_operations tc74_fops = {
			.owner = THIS_MODULE,
			.read = tc74_read_file,
			.write = tc74_write_file,
	};

	static int tc74_probe(struct i2c_client *client, const struct i2c_device_id *id){
		static int counter =0;

		struct tc74_dev *tc74;

		//allocate new structure representing current device
		tc74 = devm_kzalloc(&client->dev, sizeof(struct tc74_dev), GFP_KERNEL);

		//store pointer to the device-structure in bus device context
		i2c_set_clientdata(client, tc74);

		//store pointer to i2c device/client
		tc74->client = client;

		//initialize the misc device, tc74 incremented after each probe
		sprintf(tc74->name, "tc74%02d", counter++);
		dev_info(&client->dev, "tc74_probe is entered on %s\n", tc74->name);

		tc74->tc74_miscdevice.name = tc74->name;
		tc74->tc74_miscdevice.minor = MISC_DYNAMIC_MINOR;
		tc74->tc74_miscdevice.fops = &tc74_fops;

		//register misc device
		return misc_register(&tc74->tc74_miscdevice);

		dev_info(&client->dev,"tc74_probe is exited on %s\n", tc74->name);

		return 0;
	}

	static int tc74_remove(struct i2c_client *client){
		struct tc74_dev *tc74;

		//get device structure from bus device context
		tc74=i2c_get_clientdata(client);
		dev_info(&client->dev, "tc74_remove is entered on %s\n", tc74->name);

		//deregister misc devices
		misc_deregister(&tc74->tc74_miscdevice);

		dev_info(&client->dev, "tc74_remove is exited on %s\n", tc74->name);

		return 0;

	}

	static const struct of_device_id tc74_dt_ids[]={
		{.compatible = "tc74, microchip",},
		{}
	};

	MODULE_DEVICE_TABLE(of, tc74_dt_ids);

	static const struct i2c_device_id i2c_ids[]={
			{.name = "tc74",},
			{}
	};

	MODULE_DEVICE_TABLE(i2c,i2c_ids);

	static struct i2c_driver tc74_driver = {
		.driver = {
				.name="tc74",
				.owner=THIS_MODULE,
				.of_match_table=tc74_dt_ids,
		},
		.probe=tc74_probe,
		.remove=tc74_remove,
		.id_table=i2c_ids,
	};

	module_i2c_driver(tc74_driver);

	MODULE_LICENSE("GPL");
	MODULE_AUTHOR("Vlad Barbalata");
	MODULE_DESCRIPTION("This is a platform driver that controls a temp sensor");


