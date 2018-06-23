#ifndef _PILAB_I2C_DEVICE_H
#define _PILAB_I2C_DEVICE_H
#include "pilab-slave-device.h"
#include "pilab-host-device.h"

struct t_i2c_device;

typedef int(t_i2c_device_init_strategy)(void *arg1, void *arg2);

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
	 * Pin base should be a number > 0x40.
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
	t_i2c_device_init_strategy *callback_init_strategy;
};

extern int i2c_device_analog_read(const void *instance, int pin);
extern void i2c_device_analog_write(const void *instance, int pin, int value);
extern int i2c_device_digital_read(const void *instance, int pin);
extern void i2c_device_digital_write(const void *instance, int pin, int value);
extern void i2c_device_set_pointer(const void *instance, const char *property,
				   void *pointer);
extern int i2c_device_get_pin_base(const void *instance);
extern int i2c_device_get_expansion_pin(const void *instance, int pin);
extern const char *i2c_device_get_name(const void *instance);
extern int i2c_device_get_address(const void *instance);
extern void i2c_device_free_device(const void *instance);
extern struct t_i2c_device *i2c_device_create(int pin_base, int address,
					      const char *device_name,
					      struct t_host_device *host,
					      struct t_slave_device **slave_ref);
#endif
