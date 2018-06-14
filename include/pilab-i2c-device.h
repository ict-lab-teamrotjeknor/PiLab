#ifndef _PILAB_I2C_DEVICE_H
#define _PILAB_I2C_DEVICE_H
#include "pilab-slave-device.h"

struct t_i2c_device;

typedef int(t_i2c_device_loading_strategy)(struct t_i2c_device *device);
typedef int(t_i2c_device_read)(struct t_i2c_device *device, int pin);
typedef void(t_i2c_device_write)(struct t_i2c_device *device, int pin, int value);

struct t_i2c_device {
	/*
	 * The name of the device.
	 */
	const char *device_name;
	/*
	 * Address for the I2C address
	 *
	 * NOTE: Can be found by issuing the following commands:
	 *
	 * For I2CPORT 0: i2cdetect -y 0
	 * For I2CPORT 1: i2cdetect -y 1
	 */
	int i2c_addr;
	/*
	 * Data pin of the device.
	 *   -1: No data pin
	 * >= 0: Data pin.
	 */
	int sda_pin;
	/*
	 * Pin base should be a number >= 0x40.
	 */
	int pin_base;
	/*
	 * File descriptor for accessing the device.
	 */
	int handle;

	/* Callbacks */

	/*
	 * Callback used for loading the specific device module.
	 *
	 * NOTE: Upon initialisation of your concrete type, this should be set
	 * to NULL as implementation details might vary so much that a good
	 * default isn't possible.
	 */
	t_i2c_device_loading_strategy *callback_loading_strategy;
	/*
	 * Callback used for reading raw data from a specific pin.
	 */
	t_i2c_device_read *callback_read;
	/*
	 * Callback used for writing a value to a specific pin.
	 */
	t_i2c_device_write *callback_write;
};

#endif
