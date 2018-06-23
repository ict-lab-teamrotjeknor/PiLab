#include <unistd.h>
#include <stdlib.h>
#include <wiringPi.h>
#include "pilab-gpio-device.h"
#include "pilab-string.h"
#include "pilab-log.h"

/*
 * GPIO device's concrete analog read implementation.
 *
 * Returns the read value of the device, otherwise -99999.
 */

int gpio_device_analog_read(const void *instance, int pin)
{
	struct t_gpio_device *device;

	if (!instance)
		return -99999;

	device = (struct t_gpio_device *)instance;

	if (!device)
		return -99999;

	pinMode(pin, INPUT);

	return analogRead(pin);
}

/*
 * GPIO device's concrete analog write implementation.
 *
 * Writes a value to the specified pin of the device.
 */

void gpio_device_analog_write(const void *instance, int pin, int value)
{
	struct t_gpio_device *device;

	if (!instance)
		return;

	device = (struct t_gpio_device *)instance;

	if (!device)
		return;

	pinMode(pin, OUTPUT);

	analogWrite(pin, value);
}

/*
 * GPIO device's concrete digital read implementation.
 *
 * Returns the read value of the device, otherwise -99999.
 */

int gpio_device_digital_read(const void *instance, int pin)
{
	struct t_gpio_device *device;

	if (!instance)
		return -99999;

	device = (struct t_gpio_device *)instance;

	if (!device)
		return -99999;

	return digitalRead(pin);
}

/*
 * GPIO device's concrete digital write implementation.
 *
 * Writes a value to the specified pin of the device.
 */

void gpio_device_digital_write(const void *instance, int pin, int value)
{
	struct t_gpio_device *device;

	if (!instance)
		return;

	device = (struct t_gpio_device *)instance;

	if (!device)
		return;

	digitalWrite(pin, value);
}

/*
 * GPIO device's concrete get pin base implementation.
 *
 *
 * Returns the device's pinbase, -1 otherwise
 */

int gpio_device_get_pin_base(const void *instance)
{
	struct t_gpio_device *device;

	if (!instance)
		return -1;

	device = (struct t_gpio_device *)instance;

	return device->pin_base;
}

/*
 * GPIO device's concrete set pointer implementation.
 *
 * Set gpio device property (pointer)
 *
 * NOTE: Currently the properties that are allowed to be set:
 * - callback_init_strategy
 */

void gpio_device_set_pointer(const void *instance, const char *property,
			     void *pointer)
{
	struct t_gpio_device *device;

	if (!instance || !property)
		return;

	device = (struct t_gpio_device *)instance;

	if (string_strcmp(property, "callback_init_strategy"))
		device->callback_init_strategy = pointer;
}

/*
 * GPIO device's concrete get expander pin implementation.
 *
 * Retrieve the expansion pin device.
 *
 * Returns the expansion pin, -1 otherwise.
 */

int gpio_device_get_expansion_pin(const void *instance, int pin)
{
	struct t_gpio_device *device;

	if (!instance)
		return -1;

	device = (struct t_gpio_device *)instance;

	return device->pin_base + pin;
}

/*
 * GPIO device's concrete get name implementation.
 *
 * Returns the name of the device, NULL otherwise.
 */

const char *gpio_device_get_name(const void *instance)
{
	struct t_gpio_device *device;

	if (!instance)
		return NULL;

	device = (struct t_gpio_device *)instance;

	return device->device_name;
}

/*
 * GPIO device's concrete get address implementation.
 *
 * Returns the address of the device, -1 otherwise.
 */

int gpio_device_get_address(const void *instance)
{
	struct t_gpio_device *device;

	if (!instance)
		return -1;

	device = (struct t_gpio_device *)instance;
	/* Silence! */
	(void)device;

	/* This device has no addresses associated with it */
	return -1;
}

/*
 * GPIO device's concrete free implementation.
 */

void gpio_device_free_device(const void *instance)
{
	struct t_gpio_device *device;

	if (!instance)
		return;

	device = (struct t_gpio_device *)instance;

	free(device);
}

/*
 * Creates a new gpio device and will act as an abstract slave_device so we can add
 * it to a host device.
 *
 * NOTE: Make sure that there are no overlapping pin offsets when registering
 * the device as a extension module, as there is no boundary checking.
 *
 * Returns pointer to the newly created gpio_device, NULL otherwise.
 */

struct t_gpio_device *gpio_device_create(int pin_base, const char *device_name,
					 struct t_host_device *host)
{
	struct t_gpio_device *new_device;
	struct t_slave_device *new_slave;

	if (pin_base < 0x40) {
		pilab_log(
			LOG_DEBUG,
			"Pin base should be a number > 0x40 in gpio_device_create.");
		return NULL;
	}

	new_device = malloc(sizeof(*new_device));
	if (!new_device) {
		pilab_log(LOG_DEBUG,
			  "Could not allocate memory for new gpio device.");
		return NULL;
	}

	new_slave = malloc(sizeof(*new_slave));
	if (!new_device) {
		pilab_log(LOG_DEBUG,
			  "Could not allocate memory for new slave device.");
		return NULL;
	}

	new_device->pin_base = pin_base;
	new_device->device_name = device_name;
	new_device->callback_init_strategy = NULL;

	new_slave->get_expansion_pin = &gpio_device_get_expansion_pin;
	new_slave->get_name = &gpio_device_get_name;
	new_slave->analog_read = &gpio_device_analog_read;
	new_slave->analog_write = &gpio_device_analog_write;
	new_slave->digital_read = &gpio_device_digital_read;
	new_slave->digital_write = &gpio_device_digital_write;
	new_slave->set_pointer = &gpio_device_set_pointer;
	new_slave->get_pinbase = &gpio_device_get_pin_base;
	new_slave->free_device = &gpio_device_free_device;
	new_slave->instance = new_device;

	if (host) {
		/* register the device */
		host_device_register_slave_device(host, new_slave);
	}

	return new_device;
}
