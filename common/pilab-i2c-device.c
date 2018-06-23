#include <stdio.h>
#include <unistd.h>
#include <wiringPiI2C.h>
#include <stdlib.h>
#include "pilab-i2c-device.h"
#include "pilab-string.h"
#include "pilab-log.h"

/*
 * I2C device's concrete analog read implementation.
 *
 * NOTE: I2C doesn't differentiate between analog and digital, as such all
 * calls can to _type_read|write can be interpreted as read | write.
 *
 * Returns the read value of the device, otherwise -99999.
 */

int i2c_device_analog_read(const void *instance, int pin)
{
	struct t_i2c_device *device;

	if (!instance)
		return -99999;

	/* Silence! */
	(void)pin;

	device = (struct t_i2c_device *)instance;

	return wiringPiI2CRead(device->handle);
}

/*
 * I2C device's concrete analog write implementation.
 *
 * NOTE: I2C doesn't differentiate between analog and digital, as such all
 * calls can to _type_read|write can be interpreted as read | write.
 *
 * Writes a value to the specified pin of the device.
 */

void i2c_device_analog_write(const void *instance, int pin, int value)
{
	struct t_i2c_device *device;

	if (!instance)
		return;

	device = (struct t_i2c_device *)instance;

	/* Silence! */
	(void)pin;

	wiringPiI2CWrite (device->handle, value);
}

/*
 * I2C device's concrete get pin base implementation.
 *
 *
 * Returns the device's pinbase, -1 otherwise
 */

int i2c_device_get_pin_base(const void *instance)
{
	struct t_i2c_device *device;

	if (!instance)
		return -1;

	device = (struct t_i2c_device *)instance;

	return device->pin_base;
}

/*
 * I2C device's concrete digital read implementation.
 *
 * NOTE: I2C doesn't differentiate between analog and digital, as such all
 * calls can to _type_read|write can be interpreted as read | write.
 *
 * Returns the read value of the device, otherwise -99999.
 */

int i2c_device_digital_read(const void *instance, int pin)
{
	return i2c_device_analog_read(instance, pin);
}

/*
 * I2C device's concrete digital write implementation.
 *
 * NOTE: I2C doesn't differentiate between analog and digital, as such all
 * calls can to _type_read|write can be interpreted as read | write.
 *
 * Writes a value to the specified pin of the device.
 */

void i2c_device_digital_write(const void *instance, int pin, int value)
{
	i2c_device_analog_write(instance, pin, value);
}

/*
 * I2C device's concrete set pointer implementation.
 *
 * Set i2c device property (pointer)
 *
 * NOTE: Currently the properties that are allowed to be set:
 * - callback_init_strategy
 */

void i2c_device_set_pointer(const void *instance, const char *property,
			    void *pointer)
{
	struct t_i2c_device *device;

	if (!instance || !property)
		return;

	device = (struct t_i2c_device *)instance;

	if (string_strcmp(property, "callback_init_strategy"))
		device->callback_init_strategy = pointer;
}

/*
 * I2C device's concrete get expander pin implementation.
 *
 * Retrieve the expansion pin device.
 *
 * Returns the expansion pin, -1 otherwise.
 */

int i2c_device_get_expansion_pin(const void *instance, int pin)
{
	struct t_i2c_device *device;

	if (!instance)
		return -1;

	device = (struct t_i2c_device *)instance;

	return device->pin_base + pin;
}

/*
 * I2C device's concrete get name implementation.
 *
 * Returns the name of the device, NULL otherwise.
 */

const char *i2c_device_get_name(const void *instance)
{
	struct t_i2c_device *device;

	if (!instance)
		return NULL;

	device = (struct t_i2c_device *)instance;

	return device->device_name;
}

/*
 * I2C device's concrete get address implementation.
 *
 * Returns the address of the device, -1 otherwise.
 */

int i2c_device_get_address(const void *instance)
{
	struct t_i2c_device *device;

	if (!instance)
		return -1;

	device = (struct t_i2c_device *)instance;

	return device->i2c_addr;
}

/*
 * I2C device's concrete free implementation.
 */

void i2c_device_free_device(const void *instance)
{
	struct t_i2c_device *device;

	if (!instance)
		return;

	device = (struct t_i2c_device *)instance;

	free(device);
}

/*
 * Creates a new i2c device and will act as an abstract slave_device for a host
 * device.
 *
 * When setting up the i2c device as an extension module, make sure the
 * the pin_base is > 0x40
 *
 * NOTE: Make sure that there are no overlapping pin offsets when registering
 * the device as a extension module, as there is no boundary checking.
 *
 * Returns pointer to the newly created i2c_device, NULL otherwise.
 */

struct t_i2c_device *i2c_device_create(int pin_base, int address,
				       const char *device_name,
				       struct t_host_device *host, struct t_slave_device **slave_ref)
{
	struct t_i2c_device *new_device;
	struct t_slave_device *new_slave;

	if (pin_base < 0x40) {
		pilab_log(
			LOG_DEBUG,
			"Pin base should be a number > 0x40 in i2c_device_create.");
		return NULL;
	}

	new_device = malloc(sizeof(*new_device));
	if (!new_device) {
		pilab_log(LOG_DEBUG,
			  "Could not allocate memory for new i2c device.");
		return NULL;
	}

	new_slave = malloc(sizeof(*new_slave));
	if (!new_device) {
		pilab_log(LOG_DEBUG,
			  "Could not allocate memory for new slave device.");
		return NULL;
	}

	new_device->pin_base = pin_base;
	new_device->i2c_addr = address;
	new_device->device_name = device_name;
	new_device->callback_init_strategy = NULL;

	new_slave->get_expansion_pin = &i2c_device_get_expansion_pin;
	new_slave->get_name = &i2c_device_get_name;
	new_slave->analog_read = &i2c_device_analog_read;
	new_slave->analog_write = &i2c_device_analog_write;
	new_slave->digital_read = &i2c_device_digital_read;
	new_slave->digital_write = &i2c_device_digital_write;
	new_slave->set_pointer = &i2c_device_set_pointer;
	new_slave->get_pinbase = &i2c_device_get_pin_base;
	new_slave->free_device = &i2c_device_free_device;
	new_slave->instance = new_device;

	if (slave_ref)
		*slave_ref = new_slave;

	if (host) {
		/* register the device */
		host_device_register_slave_device(host, new_slave);
	}

	return new_device;
}
