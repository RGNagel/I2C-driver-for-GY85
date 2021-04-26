/*
 * GY-85.c
 *
 *  Created on: 26 de abr de 2021
 *      Author: rgnagel
 */

#include <linux/types.h>
#include <linux/stddef.h>

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/input-polldev.h>
#include <linux/debugfs.h>

#define true 1
#define false 0

/* create private structure */
struct GY85_dev {
	// struct i2c_client represents an I2C slave device

	struct {
		bool is_active;
		struct i2c_client *i2c_client;
		struct dentry *debugfs_dir;
		struct {
			s16 x, y, z;
		} data;
	} ADXL345;

	struct {
		bool is_active;
		struct i2c_client *i2c_client;
		struct dentry *debugfs_dir;
	} ITG3205;

	struct {
		bool is_active;
		struct i2c_client *i2c_client;
		struct dentry *debugfs_dir;
	} HMC5883L;

	struct dentry *debugfs_dir;

	struct input_polled_dev *polled_input;
};

struct GY85_dev g_GY85_dev = {
		.HMC5883L.is_active = false,
		.ITG3205.is_active = false,
		.ADXL345.is_active = false
};

/**
 * ref.: https://medium.com/jungletronics/gy-85-a-quick-datasheet-study-79019bb36fbf
 */
enum ADXL345_REGS {
	ADXL345_REG_POWER_CTL = 0x2D,
	ADXL345_REG_DATAX_LSB = 0x32,
	ADXL345_REG_DATAX_MSB = 0x33,
	ADXL345_REG_DATAY_LSB = 0x34,
	ADXL345_REG_DATAY_MSB = 0x35,
	ADXL345_REG_DATAZ_LSB = 0x36,
	ADXL345_REG_DATAZ_MSB = 0x37
};

#define PCTL_MEASURE	(1 << 3)

/* poll function */
static void GY85_poll(struct input_polled_dev *pl_dev)
{
	struct GY85_dev *dev = pl_dev->private;
	s32 val = 0;

	if (dev->ADXL345.is_active) {
		// x axis
		val = i2c_smbus_read_byte_data(dev->ADXL345.i2c_client,
				ADXL345_REG_DATAX_LSB);
		dev->ADXL345.data.x = 0xFF & val;
		val = i2c_smbus_read_byte_data(dev->ADXL345.i2c_client,
				ADXL345_REG_DATAX_MSB);
		dev->ADXL345.data.x = (0xFF & val) << 8;

		// y axis
		val = i2c_smbus_read_byte_data(dev->ADXL345.i2c_client,
				ADXL345_REG_DATAY_LSB);
		dev->ADXL345.data.y = 0xFF & val;
		val = i2c_smbus_read_byte_data(dev->ADXL345.i2c_client,
				ADXL345_REG_DATAY_MSB);
		dev->ADXL345.data.y = (0xFF & val) << 8;

		// z axis
		val = i2c_smbus_read_byte_data(dev->ADXL345.i2c_client,
				ADXL345_REG_DATAZ_LSB);
		dev->ADXL345.data.z = 0xFF & val;
		val = i2c_smbus_read_byte_data(dev->ADXL345.i2c_client,
				ADXL345_REG_DATAZ_MSB);
		dev->ADXL345.data.z = (0xFF & val) << 8;
	}

	if (dev->ITG3205.is_active) {

	}

	if (dev->HMC5883L.is_active) {

	}

	// if ((val > 0xc0) && (val < 0xff)) {
	// 	input_event(ioaccel->polled_input->input, EV_KEY, KEY_1, 1);
	// } else {
	// 	input_event(ioaccel->polled_input->input, EV_KEY, KEY_1, 0);
	// }
	// input_sync(ioaccel->polled_input->input);
}

static ssize_t read_ADXL345_x(struct file *fp, char __user *user_buffer,
		size_t count, loff_t *position)
{
	u32 written = 0;
	char buf[3];

	written = scnprintf(buf, sizeof(buf), "%i", g_GY85_dev.ADXL345.data.x);

	return simple_read_from_buffer(user_buffer, count, position, buf,
			written);
}
static ssize_t read_ADXL345_y(struct file *fp, char __user *user_buffer,
		size_t count, loff_t *position)
{
	u32 written = 0;
	char buf[3];

	written = scnprintf(buf, sizeof(buf), "%i", g_GY85_dev.ADXL345.data.y);

	return simple_read_from_buffer(user_buffer, count, position, buf,
			written);
}
static ssize_t read_ADXL345_z(struct file *fp, char __user *user_buffer,
		size_t count, loff_t *position)
{
	u32 written = 0;
	char buf[3];

	written = scnprintf(buf, sizeof(buf), "%i", g_GY85_dev.ADXL345.data.z);

	return simple_read_from_buffer(user_buffer, count, position, buf,
			written);
}

static const struct file_operations fops_read_ADXL345_x = {.read =
		read_ADXL345_x,
		.owner = THIS_MODULE};
static const struct file_operations fops_read_ADXL345_y = {.read =
		read_ADXL345_y,
		.owner = THIS_MODULE};
static const struct file_operations fops_read_ADXL345_z = {.read =
		read_ADXL345_z,
		.owner = THIS_MODULE};

static ssize_t read_attr_ADXL345_x(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%i\n", g_GY85_dev.ADXL345.data.x);
}
static ssize_t read_attr_ADXL345_y(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", g_GY85_dev.ADXL345.data.y);
}
static ssize_t read_attr_ADXL345_z(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", g_GY85_dev.ADXL345.data.z);
}

static DEVICE_ATTR(ADXL345_x, 0644, read_attr_ADXL345_x, NULL);
static DEVICE_ATTR(ADXL345_y, 0644, read_attr_ADXL345_y, NULL);
static DEVICE_ATTR(ADXL345_z, 0644, read_attr_ADXL345_z, NULL);

static struct attribute *ADXL345_attrs[] = {
		&dev_attr_ADXL345_x.attr,
		&dev_attr_ADXL345_y.attr,
		&dev_attr_ADXL345_z.attr,
		NULL
};

static struct attribute_group ADXL345_attr_group = {
		.name = "ADXL345", // directory
		.attrs = ADXL345_attrs,
};

/**
 * it is call for each i2c device on the bus that matches
 * our 'compatible' string
 */

static int GY85_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
//	if (!g_GY85_dev.debugfs_dir) {
		g_GY85_dev.debugfs_dir =
				debugfs_create_dir("GY-85",
				NULL /* /sys/kernel/debug */);
//	}

	switch (client->addr) {
	case 0x53:
		// ADXL345 -  0x53 — Three axis acceleration
		dev_info(&client->dev, "probing ADXL345\n");
		g_GY85_dev.ADXL345.i2c_client = client;
		g_GY85_dev.ADXL345.is_active = true;

		// debugfs
		g_GY85_dev.ADXL345.debugfs_dir = debugfs_create_dir("ADXL345",
				g_GY85_dev.debugfs_dir);
		if (g_GY85_dev.ADXL345.debugfs_dir) {
			debugfs_create_file("x", 0444,
					g_GY85_dev.ADXL345.debugfs_dir, NULL,
					&fops_read_ADXL345_x);
			debugfs_create_file("y", 0444,
					g_GY85_dev.ADXL345.debugfs_dir, NULL,
					&fops_read_ADXL345_y);
			debugfs_create_file("z", 0444,
					g_GY85_dev.ADXL345.debugfs_dir, NULL,
					&fops_read_ADXL345_z);
		}

		if (sysfs_create_group(&client->dev.kobj, &ADXL345_attr_group)
				!= 0)
			dev_info(&client->dev,
					"failed to create sysfs group\n");

		/* enter measurement mode */
		i2c_smbus_write_byte_data(client, ADXL345_REG_POWER_CTL,
		PCTL_MEASURE);

		break;
	case 0x69:
		// ITG3205  - 0x69 — Three axis gyroscope
		dev_info(&client->dev, "probing ITG3205\n");
		g_GY85_dev.ITG3205.i2c_client = client;
		g_GY85_dev.ITG3205.is_active = true;
		g_GY85_dev.ITG3205.debugfs_dir = debugfs_create_dir("ITG3205",
				g_GY85_dev.debugfs_dir);

		break;
	case 0x1E:
		// HMC5883L - 0x1E — Three axis magnetic field
		dev_info(&client->dev, "probing HMC5883L\n");
		g_GY85_dev.HMC5883L.i2c_client = client;
		g_GY85_dev.HMC5883L.is_active = true;
		g_GY85_dev.HMC5883L.debugfs_dir = debugfs_create_dir("HMC5883L",
				g_GY85_dev.debugfs_dir);

		break;
	}

	/* associate client->dev with gy85 private structure */
	i2c_set_clientdata(client, &g_GY85_dev);

	/* init polling mechanism if it hasn't been initialized yet. */

	static bool polling_initialized = false;
	if (polling_initialized == true)
		return 0;

	/* allocate the struct input_polled_dev */
	g_GY85_dev.polled_input = devm_input_allocate_polled_device(
			&client->dev);

	/* initialize polled input */

	g_GY85_dev.polled_input->private = &g_GY85_dev;

	g_GY85_dev.polled_input->poll_interval = 100; // ms
	g_GY85_dev.polled_input->poll = GY85_poll;

	g_GY85_dev.polled_input->input->dev.parent = &client->dev;

	g_GY85_dev.polled_input->input->name = "Read GY-85 data";
	g_GY85_dev.polled_input->input->id.bustype = BUS_I2C;

	/* set event types */
	set_bit(EV_KEY, g_GY85_dev.polled_input->input->evbit);
	set_bit(KEY_1, g_GY85_dev.polled_input->input->keybit);

	/* register the device, now the device is global until being unregistered*/
	input_register_polled_device(g_GY85_dev.polled_input);

	polling_initialized = true;
	return 0;
}

static int GY85_remove(struct i2c_client *client)
{

	switch (client->addr) {
	case 0x53:
		// ADXL345 -  0x53 — Three axis acceleration
		dev_info(&client->dev, "removing ADXL345\n");
		g_GY85_dev.ADXL345.is_active = false;
		debugfs_remove_recursive(g_GY85_dev.ADXL345.debugfs_dir);
		break;
	case 0x69:
		// ITG3205  - 0x69 — Three axis gyroscope
		dev_info(&client->dev, "removing ITG3205\n");
		g_GY85_dev.ITG3205.is_active = false;
		debugfs_remove_recursive(g_GY85_dev.ITG3205.debugfs_dir);
		break;
	case 0x1E:
		// HMC5883L - 0x1E — Three axis magnetic field
		dev_info(&client->dev, "removing HMC5883L\n");
		debugfs_remove_recursive(g_GY85_dev.HMC5883L.debugfs_dir);
		g_GY85_dev.HMC5883L.is_active = false;
		break;
	}

	if (!g_GY85_dev.HMC5883L.is_active && !g_GY85_dev.ITG3205.is_active
			&& !g_GY85_dev.ADXL345.is_active) {
		struct GY85_dev *gy85 = i2c_get_clientdata(client);
		input_unregister_polled_device(gy85->polled_input);
		debugfs_remove_recursive(g_GY85_dev.debugfs_dir);
		sysfs_remove_group(&client->dev.kobj, &ADXL345_attr_group);
	}

	return 0;
}

/*
 * device tree:
 * will match all i2c devices that have the compatible string
 */
static const struct of_device_id ioaccel_dt_ids[] = {
		{.compatible = "GY-85", },
		{}
};
MODULE_DEVICE_TABLE(of, ioaccel_dt_ids);

/* FIXME: don't know what it means exactly */
/**
 *  * @id_table: List of I2C devices supported by this driver
 */
static const struct i2c_device_id i2c_ids[] = {
		{"GY-85", 0},
		{}
};
MODULE_DEVICE_TABLE(i2c, i2c_ids);

/**
 * register our driver with i2c bus
 *
 * this driver will be list in /sys/bus/i2c/drivers/GY-85
 */

/* driver: create struct i2c_driver */
static struct i2c_driver GY85_driver = {
		.driver = {
				.name = "GY-85",
				.owner = THIS_MODULE,
				.of_match_table = ioaccel_dt_ids,
		},
		.probe = GY85_probe,
		.remove = GY85_remove,
		.id_table = i2c_ids,
};

/*
 * register to i2c bus as a driver.
 * The init/exit functions goes here
 * */
module_i2c_driver(GY85_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rafael Gustavo Nagel");
MODULE_DESCRIPTION("Module for reading i2c peripherals, namely ITG3205, "
		"ADXL345 and HMC5883L (GY-85).");
