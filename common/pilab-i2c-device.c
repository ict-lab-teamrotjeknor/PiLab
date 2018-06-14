#include <unistd.h>
#include <stdlib.h>
#include "pilab-i2c-device.h"
#include "pilab-string.h"

/*
 * I2C device's concrete read implementation.
 *
 * Returns the read value of the device, otherwise -99999.
 */

int i2c_device_read(const void *instance, int pin)
{
	struct t_i2c_device *device;

	if (!instance)
		return -99999;

	device = (struct t_i2c_device *)instance;

	return (int)(device->callback_read)(device, pin);
}

/*
 * I2C device's concrete write implementation.
 *
 * Writes a value to the specified pin of the device.
 */

void i2c_device_write(const void *instance, int pin, int value)
{
	struct t_i2c_device *device;

	if (!instance)
		return;

	device = (struct t_i2c_device *)instance;

	(void)(device->callback_write)(device, pin, value);
}

/*
 * I2C device's concrete init implementation.
 *
 * Initialise the device, effectively calling the device's specific loading
 * strategy, which will assign a handle to access the device.
 */

void i2c_device_init(const void *instance)
{
	struct t_i2c_device *device;
	int fd;

	if (!instance)
		return;

	device = (struct t_i2c_device *)instance;

	fd = (int)(device->callback_loading_strategy)(device);

	/* a handle is just a device specific file descriptor */
	device->handle = fd;
}

/*
 * I2C device's close handle implementation.
 */

void i2c_device_close_handle(const void *instance)
{
	struct t_i2c_device *device;

	if (!instance)
		return;

	device = (struct t_i2c_device *)instance;

	close(device->handle);
}

/*
 * I2C device's concrete set pointer implementation.
 *
 * Set i2c device property (pointer)
 *
 * NOTE: Currently the properties that are allowed to be set:
 * - callback_read
 * - callback_write
 */

void i2c_device_set_pointer(const void *instance, const char *property,
			    void *pointer)
{
	struct t_i2c_device *device;

	if (!instance || !property)
		return;

	device = (struct t_i2c_device *)instance;

	if (string_strcasecmp(property, "callback_read"))
		device->callback_read = pointer;
	else if (string_strcasecmp(property, "callback_write"))
		device->callback_write = pointer;
}

/*
 * I2C device's concrete get expander pin implementation.
 *
 * Retrieve the expansion pin device.
 *
 * Returns the expansion pin, 0 otherwise.
 */

int i2c_device_get_expansion_pin(const void *instance, int pin)
{
	struct t_i2c_device *device;

	if (!instance)
		return 0;

	device = (struct t_i2c_device *)instance;

	return device->pin_base + pin;
}

/*
 * Creates a new i2c device and will add it as an abstract slave_device to the
 * specified host_device.
 *
 * When setting up the i2c device as an extension module, make sure the
 * the pin_base is >= 0x40. If you wish to create a * new device without setting
 * it up as an extension module, set pin_base to 0x0.
 *
 * NOTE: Make sure that there are no overlapping pin offsets when registering
 * the device as a extension module, as there is no boundary checking.
 *
 * Returns pointer to the newly created i2c_device, NULL otherwise.
 */

struct t_i2c_device *i2c_device_create(
	int pin_base, int address, const char *device_name,
	t_i2c_device_loading_strategy *callback_loading_strategy,
	t_i2c_device_read *callback_read, t_i2c_device_write *callback_write)
{
	struct t_i2c_device *new_device;
	struct t_slave_device *new_slave;

	if (!callback_loading_strategy)
		return NULL;

	/* TODO: Log message: Pin base should be a number >= 0x64 */
	if (pin_base < 0x40)
		return NULL;

	new_device = malloc(sizeof(*new_device));

	/* TODO: Log error: Could not allocate memory for new i2c device */
	if (!new_device)
		return NULL;

	new_slave = malloc(sizeof(*new_slave));

	if (!new_slave) {
		free(new_device);
		return NULL;
	}

	new_device->pin_base = pin_base;
	new_device->i2c_addr = address;
	new_device->device_name = device_name;
	new_device->callback_loading_strategy = callback_loading_strategy;
	new_device->callback_read = callback_read;
	new_device->callback_write = callback_write;

	new_slave->read = &i2c_device_read;
	new_slave->write = &i2c_device_write;
	new_slave->close_handle = &i2c_device_close_handle;

	return new_device;
}
