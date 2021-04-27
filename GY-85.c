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
		struct {
			s16 x, y, z, temp;
		} data;
	} ITG3205;

	struct {
		bool is_active;
		struct i2c_client *i2c_client;
		struct dentry *debugfs_dir;
		struct {
			s16 x, y, z;
		} data;
	} HMC5883L;

	struct dentry *debugfs_dir;

	struct input_polled_dev *polled_input;
};

struct GY85_dev g_GY85_dev = {
		.HMC5883L.is_active = false,
		.HMC5883L.data = {0},
		.ITG3205.is_active = false,
		.ITG3205.data = {0},
		.ADXL345.is_active = false,
		.ADXL345.data = {0},
		.debugfs_dir = NULL,
		.polled_input = NULL,

};

enum ADDRESSES_I2C_DEVICES {
	ADDR_I2C_ADXL345 = 0x53,
	ADDR_I2C_ITG3205 = 0x69,
	ADDR_I2C_HMC5883L = 0x1E
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

/**
 * ref.: https://www.tinyosshop.com/datasheet/itg3205.pdf
 * 
 */
enum ITG3205_REGS {
	ITG3205_REG_TEMP_H = 0x1B,
	ITG3205_REG_TEMP_L = 0x1C,
	ITG3205_REG_X_H = 0x1D,
	ITG3205_REG_X_L = 0x1E,
	ITG3205_REG_Y_H = 0x1F,
	ITG3205_REG_Y_L = 0x20,
	ITG3205_REG_Z_H = 0x21,
	ITG3205_REG_Z_L = 0x22,
	ITG3205_POWER_MANAGEMENT = 0x3E
};

/**
 * ref.: http://www.farnell.com/datasheets/1683374.pdf
 */
enum HMC5883L_REGS {
	HMC5883L_REG_MODE = 2,
	HMC5883L_REG_X_MSB,
	HMC5883L_REG_X_LSB,
	HMC5883L_REG_Z_MSB,
	HMC5883L_REG_Z_LSB,
	HMC5883L_REG_Y_MSB,
	HMC5883L_REG_Y_LSB
};

#define PCTL_MEASURE	(1 << 3)

/**
 * @brief convert MSB and LSB bytes to signed 16-bit two's complement 
 * 
 * @param MSB most significant byte
 * @param LSB least significant byte
 * @return s16 signed 16-bit two's complemen
 */
static s16 to_s16(u8 MSB, u8 LSB)
{
	return (MSB << 8) | LSB;
}

/* poll function */
static void GY85_poll(struct input_polled_dev *pl_dev)
{
	struct GY85_dev *dev = pl_dev->private;
	s32 msb = 0, lsb = 0;

	if (dev->ADXL345.is_active) {
		// x axis
		lsb = i2c_smbus_read_byte_data(dev->ADXL345.i2c_client,
				ADXL345_REG_DATAX_LSB);
		msb = i2c_smbus_read_byte_data(dev->ADXL345.i2c_client,
				ADXL345_REG_DATAX_MSB);
		if (msb >= 0 && lsb >= 0)
			dev->ADXL345.data.x = to_s16(msb, lsb);

		// y axis
		lsb = i2c_smbus_read_byte_data(dev->ADXL345.i2c_client,
				ADXL345_REG_DATAY_LSB);
		msb = i2c_smbus_read_byte_data(dev->ADXL345.i2c_client,
				ADXL345_REG_DATAY_MSB);
		if (msb >= 0 && lsb >= 0)
			dev->ADXL345.data.y = to_s16(msb, lsb);

		// z axis
		lsb = i2c_smbus_read_byte_data(dev->ADXL345.i2c_client,
				ADXL345_REG_DATAZ_LSB);
		msb = i2c_smbus_read_byte_data(dev->ADXL345.i2c_client,
				ADXL345_REG_DATAZ_MSB);
		if (msb >= 0 && lsb >= 0)
			dev->ADXL345.data.z = to_s16(msb, lsb);
	}

	if (dev->ITG3205.is_active) {
		// x axis
		msb = i2c_smbus_read_byte_data(dev->ITG3205.i2c_client,
				ITG3205_REG_X_H);
		lsb = i2c_smbus_read_byte_data(dev->ITG3205.i2c_client,
				ITG3205_REG_X_L);
		if (msb >= 0 && lsb >= 0)
			dev->ITG3205.data.x = to_s16(msb, lsb);

		// y axis
		msb = i2c_smbus_read_byte_data(dev->ITG3205.i2c_client,
				ITG3205_REG_Y_H);
		lsb = i2c_smbus_read_byte_data(dev->ITG3205.i2c_client,
				ITG3205_REG_Y_L);
		if (msb >= 0 && lsb >= 0)
			dev->ITG3205.data.y = to_s16(msb, lsb);

		// z axis
		msb = i2c_smbus_read_byte_data(dev->ITG3205.i2c_client,
				ITG3205_REG_Z_H);
		lsb = i2c_smbus_read_byte_data(dev->ITG3205.i2c_client,
				ITG3205_REG_Z_L);
		if (msb >= 0 && lsb >= 0)
			dev->ITG3205.data.z = to_s16(msb, lsb);

		// temperature
		msb = i2c_smbus_read_byte_data(dev->ITG3205.i2c_client,
				ITG3205_REG_TEMP_H);
		lsb = i2c_smbus_read_byte_data(dev->ITG3205.i2c_client,
				ITG3205_REG_TEMP_L);
		if (msb >= 0 && lsb >= 0)
			dev->ITG3205.data.temp = to_s16(msb, lsb);
	}

	if (dev->HMC5883L.is_active) {
		// x axis
		msb = i2c_smbus_read_byte_data(dev->HMC5883L.i2c_client,
				HMC5883L_REG_X_MSB);
		lsb = i2c_smbus_read_byte_data(dev->HMC5883L.i2c_client,
				HMC5883L_REG_X_LSB);
		if (msb >= 0 && lsb >= 0)
			dev->HMC5883L.data.x = to_s16(msb, lsb);

		// y axis
		msb = i2c_smbus_read_byte_data(dev->HMC5883L.i2c_client,
				HMC5883L_REG_Y_MSB);
		lsb = i2c_smbus_read_byte_data(dev->HMC5883L.i2c_client,
				HMC5883L_REG_Y_LSB);
		if (msb >= 0 && lsb >= 0)
			dev->HMC5883L.data.y = to_s16(msb, lsb);

		// z axis
		msb = i2c_smbus_read_byte_data(dev->HMC5883L.i2c_client,
				HMC5883L_REG_Z_MSB);
		lsb = i2c_smbus_read_byte_data(dev->HMC5883L.i2c_client,
				HMC5883L_REG_Z_LSB);
		if (msb >= 0 && lsb >= 0)
			dev->HMC5883L.data.z = to_s16(msb, lsb);
	}
}

/**
 * ADXL345 debugfs operations 
 */
static ssize_t read_ADXL345_x(struct file *fp, char __user *user_buffer,
		size_t count, loff_t *position)
{
	u32 written = 0;
	char buf[15];

	written = scnprintf(buf, sizeof(buf), "%i\n",
			g_GY85_dev.ADXL345.data.x);

	return simple_read_from_buffer(user_buffer, count, position, buf,
			written);
}
static ssize_t read_ADXL345_y(struct file *fp, char __user *user_buffer,
		size_t count, loff_t *position)
{
	u32 written = 0;
	char buf[15];

	written = scnprintf(buf, sizeof(buf), "%i\n",
			g_GY85_dev.ADXL345.data.y);

	return simple_read_from_buffer(user_buffer, count, position, buf,
			written);
}
static ssize_t read_ADXL345_z(struct file *fp, char __user *user_buffer,
		size_t count, loff_t *position)
{
	u32 written = 0;
	char buf[15];

	written = scnprintf(buf, sizeof(buf), "%i\n",
			g_GY85_dev.ADXL345.data.z);

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

/**
 * ITG3205 debugfs operations 
 */
static ssize_t read_ITG3205_x(struct file *fp, char __user *user_buffer,
		size_t count, loff_t *position)
{
	u32 written = 0;
	char buf[15];

	written = scnprintf(buf, sizeof(buf), "%i\n",
			g_GY85_dev.ITG3205.data.x);

	return simple_read_from_buffer(user_buffer, count, position, buf,
			written);
}
static ssize_t read_ITG3205_y(struct file *fp, char __user *user_buffer,
		size_t count, loff_t *position)
{
	u32 written = 0;
	char buf[15];

	written = scnprintf(buf, sizeof(buf), "%i\n",
			g_GY85_dev.ITG3205.data.y);

	return simple_read_from_buffer(user_buffer, count, position, buf,
			written);
}
static ssize_t read_ITG3205_z(struct file *fp, char __user *user_buffer,
		size_t count, loff_t *position)
{
	u32 written = 0;
	char buf[15];

	written = scnprintf(buf, sizeof(buf), "%i\n",
			g_GY85_dev.ITG3205.data.z);

	return simple_read_from_buffer(user_buffer, count, position, buf,
			written);
}
static ssize_t read_ITG3205_temp(struct file *fp, char __user *user_buffer,
		size_t count, loff_t *position)
{
	u32 written = 0;
	char buf[15];

	written = scnprintf(buf, sizeof(buf), "%i\n",
			g_GY85_dev.ITG3205.data.temp);

	return simple_read_from_buffer(user_buffer, count, position, buf,
			written);
}

static const struct file_operations fops_read_ITG3205_x = {.read =
		read_ITG3205_x,
		.owner = THIS_MODULE};
static const struct file_operations fops_read_ITG3205_y = {.read =
		read_ITG3205_y,
		.owner = THIS_MODULE};
static const struct file_operations fops_read_ITG3205_z = {.read =
		read_ITG3205_z,
		.owner = THIS_MODULE};
static const struct file_operations fops_read_ITG3205_temp = {.read =
		read_ITG3205_temp,
		.owner = THIS_MODULE};

/**
 * HMC5883L debugfs operations
 */
static ssize_t read_HMC5883L_x(struct file *fp, char __user *user_buffer,
		size_t count, loff_t *position)
{
	u32 written = 0;
	char buf[15];

	written = scnprintf(buf, sizeof(buf), "%i\n",
			g_GY85_dev.HMC5883L.data.x);

	return simple_read_from_buffer(user_buffer, count, position, buf,
			written);
}
static ssize_t read_HMC5883L_y(struct file *fp, char __user *user_buffer,
		size_t count, loff_t *position)
{
	u32 written = 0;
	char buf[15];

	written = scnprintf(buf, sizeof(buf), "%i\n",
			g_GY85_dev.HMC5883L.data.y);

	return simple_read_from_buffer(user_buffer, count, position, buf,
			written);
}
static ssize_t read_HMC5883L_z(struct file *fp, char __user *user_buffer,
		size_t count, loff_t *position)
{
	u32 written = 0;
	char buf[15];

	written = scnprintf(buf, sizeof(buf), "%i\n",
			g_GY85_dev.HMC5883L.data.z);

	return simple_read_from_buffer(user_buffer, count, position, buf,
			written);
}

static const struct file_operations fops_read_HMC5883L_x = {.read =
		read_HMC5883L_x,
		.owner = THIS_MODULE};
static const struct file_operations fops_read_HMC5883L_y = {.read =
		read_HMC5883L_y,
		.owner = THIS_MODULE};
static const struct file_operations fops_read_HMC5883L_z = {.read =
		read_HMC5883L_z,
		.owner = THIS_MODULE};

/**
 * it is call for each i2c device on the bus that matches
 * our 'compatible' string
 */

static int GY85_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	if (!g_GY85_dev.debugfs_dir) {
		g_GY85_dev.debugfs_dir =
				debugfs_create_dir("GY-85",
				NULL /* /sys/kernel/debug */);
	}

	switch (client->addr) {
	case ADDR_I2C_ADXL345:
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
		else {
			dev_info(&client->dev,
					"failed to create debugfs dir for ADXL\n");
		}

		/* enter measurement mode */
		i2c_smbus_write_byte_data(client, ADXL345_REG_POWER_CTL,
		PCTL_MEASURE);

		break;
	case ADDR_I2C_ITG3205:
		// ITG3205  - 0x69 — Three axis gyroscope
		dev_info(&client->dev, "probing ITG3205\n");
		g_GY85_dev.ITG3205.i2c_client = client;
		g_GY85_dev.ITG3205.is_active = true;
		g_GY85_dev.ITG3205.debugfs_dir = debugfs_create_dir("ITG3205",
				g_GY85_dev.debugfs_dir);
		if (g_GY85_dev.ITG3205.debugfs_dir) {
			debugfs_create_file("x", 0444,
					g_GY85_dev.ITG3205.debugfs_dir, NULL,
					&fops_read_ITG3205_x);
			debugfs_create_file("y", 0444,
					g_GY85_dev.ITG3205.debugfs_dir, NULL,
					&fops_read_ITG3205_y);
			debugfs_create_file("z", 0444,
					g_GY85_dev.ITG3205.debugfs_dir, NULL,
					&fops_read_ITG3205_z);
			debugfs_create_file("temp", 0444,
					g_GY85_dev.ITG3205.debugfs_dir, NULL,
					&fops_read_ITG3205_temp);
		}
		else {
			dev_info(&client->dev,
					"failed to create debugfs dir for ITG3205\n");
		}

		/* enter normal mode */
		i2c_smbus_write_byte_data(client, ITG3205_POWER_MANAGEMENT,
				0x00);

		break;

	case ADDR_I2C_HMC5883L:
		// HMC5883L - 0x1E — Three axis magnetic field
		dev_info(&client->dev, "probing HMC5883L\n");
		g_GY85_dev.HMC5883L.i2c_client = client;
		g_GY85_dev.HMC5883L.is_active = true;
		g_GY85_dev.HMC5883L.debugfs_dir = debugfs_create_dir("HMC5883L",
				g_GY85_dev.debugfs_dir);
		if (g_GY85_dev.HMC5883L.debugfs_dir) {
			debugfs_create_file("x", 0444,
					g_GY85_dev.HMC5883L.debugfs_dir, NULL,
					&fops_read_HMC5883L_x);
			debugfs_create_file("y", 0444,
					g_GY85_dev.HMC5883L.debugfs_dir, NULL,
					&fops_read_HMC5883L_y);
			debugfs_create_file("z", 0444,
					g_GY85_dev.HMC5883L.debugfs_dir, NULL,
					&fops_read_HMC5883L_z);
		}
		else {
			dev_info(&client->dev,
					"failed to create debugfs dir for HMC5883L\n");
		}

		/**
		 * enter continuous mode
		 */
		i2c_smbus_write_byte_data(client, HMC5883L_REG_MODE, 0x00);
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
	}

	return 0;
}

/*
 * device tree:
 * will match all i2c devices that have the compatible string
 */
static const struct of_device_id GY85_dt_ids[] = {
		{.compatible = "GY-85", },
		{}
};
MODULE_DEVICE_TABLE(of, GY85_dt_ids);

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
				.of_match_table = GY85_dt_ids,
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
