/*
 * GY-85.c
 *
 *  Created on: 26 de abr de 2021
 *      Author: rgnagel
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/input-polldev.h>

/* create private structure */
struct GY85_dev {
	// struct i2c_client represents an I2C slave device
	struct i2c_client * i2c_client;

	struct i2c_client *ADXL345;
	struct i2c_client *ITG3205;
	struct i2c_client *HMC5883L;

	struct input_polled_dev * polled_input;
};

//static struct GY85_dev g_GY85_dev;

#define POWER_CTL	0x2D
#define PCTL_MEASURE	(1 << 3)
#define OUT_X_MSB	0x33

/* poll function */
static void ioaccel_poll(struct input_polled_dev * pl_dev)
{
	struct GY85_dev * ioaccel = pl_dev->private;
	int val = 0;
	val = i2c_smbus_read_byte_data(ioaccel->i2c_client, OUT_X_MSB);


	if ( (val > 0xc0) && (val < 0xff) ) {
		input_event(ioaccel->polled_input->input, EV_KEY, KEY_1, 1);
	} else {
		input_event(ioaccel->polled_input->input, EV_KEY, KEY_1, 0);
	}

	input_sync(ioaccel->polled_input->input);
}

/**
 * it is call for each i2c device on the bus that matches
 * our 'compatible' string
 */

static int ioaccel_probe(struct i2c_client * client,
		const struct i2c_device_id * id)
{
	/* declare an instance of the private structure */
	struct GY85_dev * ioaccel;

	dev_info(&client->dev, "my_probe() function is called.\n");

	/* allocate private structure for new device */
	ioaccel = devm_kzalloc(&client->dev, sizeof(struct GY85_dev), GFP_KERNEL);

	/* associate client->dev with ioaccel private structure */
	i2c_set_clientdata(client, ioaccel);

	/* enter measurement mode */
	i2c_smbus_write_byte_data(client, POWER_CTL, PCTL_MEASURE);

	/* allocate the struct input_polled_dev */
	ioaccel->polled_input = devm_input_allocate_polled_device(&client->dev);

	/* initialize polled input */
	ioaccel->i2c_client = client;
	ioaccel->polled_input->private = ioaccel;

	ioaccel->polled_input->poll_interval = 50; // ms
	ioaccel->polled_input->poll = ioaccel_poll;

	ioaccel->polled_input->input->dev.parent = &client->dev;

	ioaccel->polled_input->input->name = "IOACCEL keyboard";
	ioaccel->polled_input->input->id.bustype = BUS_I2C;

	/* set event types */
	set_bit(EV_KEY, ioaccel->polled_input->input->evbit);
	set_bit(KEY_1, ioaccel->polled_input->input->keybit);

	/* register the device, now the device is global until being unregistered*/
	input_register_polled_device(ioaccel->polled_input);

	return 0;
}

static int ioaccel_remove(struct i2c_client * client)
{
	struct GY85_dev * ioaccel;
	ioaccel = i2c_get_clientdata(client);
	input_unregister_polled_device(ioaccel->polled_input);
	dev_info(&client->dev, "ioaccel_remove()\n");
	return 0;
}

/*
 * device tree:
 * will match all i2c devices that have the compatible string
 */
static const struct of_device_id ioaccel_dt_ids[] = {
	{ .compatible = "GY-85", },
	{ }
};
MODULE_DEVICE_TABLE(of, ioaccel_dt_ids);

/* FIXME: don't know what it means exactly */
/**
 *  * @id_table: List of I2C devices supported by this driver
 */
static const struct i2c_device_id i2c_ids[] = {
	{ "GY-85", 0 },
	{ }
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
	.probe = ioaccel_probe,
	.remove = ioaccel_remove,
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
