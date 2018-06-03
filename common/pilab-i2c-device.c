#include <stdlib.h>
#include "pilab-i2c-device.h"
#include "pilab-list.h"

static struct t_pilist *registration_list;

/*
 * Registers a new i2c device, effectively  adding it to the
 * `registration_list'.
 */

void i2c_device_register(struct t_i2c_device *device)
{
	if (!device)
		return;

	pilist_add(registration_list, device);
}

/*
 * Deregister a i2c device, effectively removing it from the
 * `registration_list'.
 */

void i2c_device_deregister(struct t_i2c_device *device)
{
	if (!device)
		return;

	pilist_remove_data(registration_list, device);
}

/*
 * Create a new i2c device.
 *
 * Returns pointer to the newly created i2c device, NULL otherwise.
 */

struct t_i2c_device *i2c_device_create_with_strategy(
	t_i2c_device_loading_strategy *loading_strategy)
{
	struct t_i2c_device *new_device;

	new_device = malloc(sizeof(*new_device));

	/* TODO: Log error: Could not allocated memory for new device */
	if (!new_device)
		return NULL;

	new_device->i2c_addr = 0;
	new_device->pin_base = 0;
	new_device->loading_strategy = loading_strategy;

	return new_device;
}

/*
 * Create a new i2c device without specifying a loading strategy.
 *
 * Returns pointer to the newly created i2c device, NULL otherwise.
 */

struct t_i2c_device *i2c_device_create()
{
	return i2c_device_create_with_strategy(NULL);
}

/*
 * Set/update the loading strategy of a particular i2c device.
 */

void i2c_device_set_loading_strategy(
	struct t_i2c_device *device,
	t_i2c_device_loading_strategy *callback_loading_stategy)
{
	if (!device || !callback_loading_stategy)
		return;

	device->loading_strategy = callback_loading_stategy;
}

/*
 * What is born will eventually die.
 */

void i2c_device_free(struct t_i2c_device *device)
{
	if (!device)
		return;

	free(device);
}

/*
 * Calculate the extension module pin from the pin base.
 *
 * NOTE: Call this method only if you need to interact with devices setup as an
 * extension module.
 *
 * Returns the an integer representing our new pin.
 */

int i2c_device_ext_pin(struct t_i2c_device *device, int pin)
{
	/* TODO: Log event: pin error device not found <06-05-18, Sjors Sparreboom> */
	if (!device)
		return -1;

	return (device->pin_base + pin);
}
