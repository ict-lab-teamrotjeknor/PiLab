#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include <ds18b20.h>
#include <pcf8574.h>
#include "pilab-host-device.h"
#include "pilab-gpio-device.h"
#include "pilab-i2c-device.h"
#include "pilab-hashtable.h"
#include "pilab-list.h"
#include "pilab-readline.h"
#include "pilab-log.h"
#include "pilab-string.h"
#include "pilab-lcd.h"

static const char *sensor_config_paths[] = {
	SYSCONFDIR "/pilab/sensors",
};

char *host_device_sensor_type_string[HOST_DEVICE_NUM_TYPES] = {
	PILAB_HOST_DEVICE_GPIO,
	PILAB_HOST_DEVICE_I2C,
	PILAB_HOST_DEVICE_LCD_I2C,
};

char *host_device_sensor_module[HOST_DEVICE_NUM_MODULES] = {
	PILAB_HOST_MODULE_DS18B20,
	PILAB_HOST_MODULE_PCF8574,
	PILAB_HOST_MODULE_HD44780,
};

/*
 * Search for sensor module.
 *
 * Return index of type, -1 if the type could not be found.
 */

int host_device_get_sensor_module(const char *type)
{
	if (!type)
		return -1;

	for (int i = 0; i < HOST_DEVICE_NUM_MODULES; ++i)
		if (string_strcmp(host_device_sensor_module[i], type) == 0)
			return i;

	/* type was not found */
	return -1;
}

/*
 * Search for sensor field types.
 *
 * Return index of type, -1 if the type could not be found.
 */

int host_device_get_sensor_type(const char *type)
{
	if (!type)
		return -1;

	for (int i = 0; i < HOST_DEVICE_NUM_TYPES; ++i)
		if (string_strcmp(host_device_sensor_type_string[i], type) == 0)
			return i;

	/* type was not found */
	return -1;
}

/*
 * Set callback function to free values in slave_device_lookup.
 */

void host_device_free_device_default_cb(struct t_hashtable *hashtable,
					const void *key, void *value)
{
	struct t_slave_device *device;
	if (!hashtable || !value)
		return;

	/* Silence! */
	(void)key;

	device = (struct t_slave_device *)value;

	device->free_device(device->instance);
}

/*
 * Set callback function to free keys in slave_device_lookup.
 */

void host_device_free_name_default_cb(struct t_hashtable *hashtable, void *key)
{
	if (!hashtable || !key)
		return;

	free(key);
}

/*
 * Register the module with the host device.
 */

void host_device_module_register(struct t_host_device *host_device,
				 char *sensor_name, int pin_base, char *addr)
{
	struct t_i2c_device *i2c_device;
	struct t_gpio_device *gpio_device;
	struct t_slave_device *slave_ref;
	struct t_lcd *lcd;
	int four_bit[4];
	int valid_module;
	int addr_int;

	/* TODO: initialisation should be done through the init callback,
	   as the initialisation functions can have a different amount of arguments, as
	   well as different argument types, we need a way to safely let them be executed
	   by callback_init_strategy.

	   Possible solution?: let the callback_init_strategy take a void*, which can be
	   used to cast to the function pointer in question.
	 */
	valid_module = host_device_get_sensor_module(sensor_name);
	switch (valid_module) {
	case HOST_DEVICE_MODULE_DS18B20:
		gpio_device =
			gpio_device_create(pin_base, sensor_name, host_device);
		pilab_log(LOG_DEBUG, "Setting up %s with pinbase: %d",
			  gpio_device->device_name, gpio_device->pin_base);
		ds18b20Setup(pin_base, addr);
		/* TODO: Figure out a way to do through the callback */
		/* gpio_device_set_pointer(gpio_device, "callback_init_strategy", */
		/* 			&ds18b20Setup); */
		break;
	case HOST_DEVICE_MODULE_PCF8574:
		addr_int = (int)strtol(addr, NULL, 16);
		i2c_device = i2c_device_create(pin_base, addr_int, sensor_name,
					       host_device, NULL);
		pilab_log(LOG_DEBUG,
			  "Setting up %s with pinbase: %d and address: %d",
			  i2c_device->device_name, i2c_device->pin_base,
			  i2c_device->i2c_addr);
		pcf8574Setup(pin_base, addr_int);
		/* TODO: Figure out a way to do through the callback */
		/* i2c_device_set_pointer(i2c_device, "callback_init_strategy", */
		/* 		       &pcf8574Setup); */
		break;
	case HOST_DEVICE_MODULE_HD44780:
		addr_int = (int)strtol(addr, NULL, 16);
		i2c_device = i2c_device_create(pin_base, addr_int, sensor_name,
					       NULL, &slave_ref);
		pilab_log(LOG_DEBUG,
			  "Setting up %s with pinbase: %d and address: %d",
			  i2c_device->device_name, i2c_device->pin_base,
			  i2c_device->i2c_addr);
		pcf8574Setup(pin_base, addr_int);
		/* TODO: Figure out a way to do through the callback */
		/* i2c_device_set_pointer(i2c_device, "callback_init_strategy", */
		/* 		       &pcf8574Setup); */
		four_bit[0] = i2c_device_get_expansion_pin(i2c_device, 4);
		four_bit[0] = i2c_device_get_expansion_pin(i2c_device, 5);
		four_bit[0] = i2c_device_get_expansion_pin(i2c_device, 6);
		four_bit[0] = i2c_device_get_expansion_pin(i2c_device, 7);
		int rs = i2c_device_get_expansion_pin(i2c_device, 0);
		int en = i2c_device_get_expansion_pin(i2c_device, 2);
		pilab_log(LOG_DEBUG, "Creating new lcd device");
		lcd = lcd_4bit_create(4, 20, rs, en, four_bit);
		lcd_assign_expander_chip(lcd, slave_ref);
		host_device->lcd = lcd;
		break;
	case HOST_DEVICE_NUM_MODULES:;
	}
}

/*
 * Validate the sensor module components and add it to the slave device lookup of
 * the host.
 *
 * Returns:
 * -1: invalid argument
 *  0: on error.
 *  1: on success;
 */

int host_device_slave_builder(struct t_host_device *host_device,
			      struct t_pilist *slave_components)
{
	int valid_type;
	int pin_base;
	char *sensor_name, *type;

	if (!host_device || !slave_components)
		return -1;

	sensor_name = (char *)pilist_get_data(slave_components, 0);
	/* gpio */
	type = (char *)pilist_get_data(slave_components, 1);
	/* pin base */
	pin_base = atoi((char *)pilist_get_data(slave_components, 2));

	valid_type = host_device_get_sensor_type(type);

	if (valid_type > -1) {
		switch (valid_type) {
		case HOST_DEVICE_GPIO:
			host_device_module_register(
				host_device, sensor_name, pin_base,
				(char *)pilist_get_data(slave_components, 3));
			break;
		case HOST_DEVICE_I2C:
			/* this should be safe as every i2c device should have an address */
			if (slave_components->size < 4) {
				pilab_log(
					LOG_ERROR,
					"An i2c device should have at least four columns: <name>, <type>, <pinbase>, <address>. Please make sure the sensor config is properly configured and aligned!");
				return 0;
			}
			host_device_module_register(
				host_device, sensor_name, pin_base,
				(char *)pilist_get_data(slave_components, 3));
			break;
		case HOST_DEVICE_LCD_I2C:
			/* this should be safe as every i2c device should have an address */
			if (slave_components->size < 4) {
				pilab_log(
					LOG_ERROR,
					"An lcd device should have and i2c device attached, therefore should have at least four columns: <name>, <type>, <pinbase>, <address>. Please make sure the sensor config is properly configured and aligned!");
				return 0;
			}
			host_device_module_register(
				host_device, sensor_name, pin_base,
				(char *)pilist_get_data(slave_components, 3));
			break;
		case HOST_DEVICE_NUM_TYPES:;
		}
	}

	return 1;
}

/*
 * Create a new host device, with a max specified registration size.
 *
 * Returns a pointer to the newly created host device, NULL otherwise.
 */

struct t_host_device *host_device_create_with_max_registrations_size(int size)
{
	struct t_host_device *new_host;
	struct t_hashtable *new_slave_device_lookup_table;

	new_host = malloc(sizeof(*new_host));

	if (!new_host)
		return NULL;

	/* Default to 26, as the model 3B+ has 26 programmable pins */
	new_slave_device_lookup_table =
		hashtable_create(size, PILAB_HASHTABLE_STRING,
				 PILAB_HASHTABLE_POINTER, NULL, NULL);

	if (!new_slave_device_lookup_table) {
		free(new_host);
		return NULL;
	}

	new_host->slave_devices_lookup = new_slave_device_lookup_table;
	hashtable_set_pointer(new_host->slave_devices_lookup,
			      "callback_free_value",
			      &host_device_free_device_default_cb);
	hashtable_set_pointer(new_host->slave_devices_lookup,
			      "callback_free_key",
			      &host_device_free_name_default_cb);

	return new_host;
}

/*
 * Create a new host device, with a default specified registration size of 26.
 *
 * NOTE: The number 26 represents the 26 programmable data pins for the Raspberry Pi 3B+
 *
 * Returns a pointer to the newly created host device, NULL otherwise.
 */

struct t_host_device *host_device_create()
{
	return host_device_create_with_max_registrations_size(26);
}

/*
 * Register a new slave device, effectively adding it to the host.
 *
 * The device should be registered under a unique name, as existing names will
 * be overwritten by the new device.
 */

void host_device_register_slave_device(struct t_host_device *host_device,
				       const struct t_slave_device *slave_device)
{
	if (!host_device || !slave_device)
		return;

	hashtable_set(host_device->slave_devices_lookup,
		      slave_device->get_name(slave_device->instance),
		      slave_device);
}

/*
 * Deregister slave device, effectively decoupling it from the host.
 */

void host_device_deregister_slave_device(
	struct t_host_device *host_device,
	const struct t_slave_device *slave_device)
{
	if (!host_device || !slave_device)
		return;

	hashtable_remove(host_device->slave_devices_lookup,
			 slave_device->get_name(slave_device->instance));
}

/*
 * Deregister all slave devices currently attached to the host.
 */

void host_device_deregister_all_slave_devices(struct t_host_device *host_device)
{
	if (!host_device)
		return;

	hashtable_remove_all(host_device->slave_devices_lookup);
}

/*
 * Returns the total amount of registered slave devices, -1 otherwise.
 */

int host_device_get_slave_devices_count(struct t_host_device *host_device)
{
	if (!host_device)
		return -1;

	return host_device->slave_devices_lookup->count;
}

/*
 * Read in the sensors and initialise them.
 */

void host_device_read_in_sensor_modules(struct t_host_device *host_device)
{
	char *line;
	FILE *file;
	int line_no;
	struct t_pilist *split_sensor_line;

	if (!host_device)
		return;

	line_no = 0;
	file = fopen(sensor_config_paths[0], "r");
	if (file) {
		while (!feof(file)) {
			line = read_line(file);

			if (!line)
				continue;

			line_no++;
			pilab_log(LOG_DEBUG, "Read line %d: %s", line_no, line);
			line = string_strip_whitespace(line);

			/* this is a comment, skip it */
			if (*line == '#') {
				free(line);
				continue;
			}

			/* empty is just empty */
			if (strlen(line) == 0) {
				free(line);
				continue;
			}

			split_sensor_line = NULL;
			/* we split on whitespace */
			split_sensor_line = string_split(line, " ");
			if (split_sensor_line) {
				if (host_device_slave_builder(
					    host_device, split_sensor_line)) {
					free(split_sensor_line);
					free(line);
					split_sensor_line = NULL;
					line = NULL;
				} else {
					pilab_log(LOG_DEBUG,
						  "Could not split line %d: %s",
						  line_no, line);
				}
			} else {
				pilab_log(
					LOG_DEBUG,
					"Could not split line for sensors...");
			}
		}
	} else {
		pilab_log(LOG_ERROR, "%s", "Could not open sensor file");
	}

	/* whats open needs to be closed */
	fclose(file);
}

/*
 * Get the sensor names in a list, which can be used for lookup purposes.
 *
 * NOTE: The list returned needs to be freed afterwards.
 *
 * Returns a list with sensor names for this host, NULL otherwise.
 */

struct t_pilist *
	host_device_get_sensor_name_list(struct t_host_device *host_device)
{
	struct t_pilist *key_list;

	if (!host_device)
		return NULL;

	key_list = hashtable_get_key_list(host_device->slave_devices_lookup);

	return key_list;
}

/*
 * Get the sensors in a list.
 *
 * NOTE: The list returned needs to be freed afterwards.
 *
 * Returns a list with all the sensors for this host, NULL otherwise.
 */

struct t_pilist *host_device_get_sensor_list(struct t_host_device *host_device)
{
	struct t_pilist *key_list;
	struct t_pilist *sensor_list;
	struct t_slave_device *sensor;
	char *sensor_name;
	int i;

	if (!host_device)
		return NULL;

	sensor_list = pilist_create();

	if (!sensor_list) {
		pilab_log(LOG_DEBUG, "Could not allocate memory for pilist");
		return NULL;
	}

	key_list = host_device_get_sensor_name_list(host_device);

	if (!key_list) {
		pilab_log(LOG_DEBUG, "Received empty key list");
		return NULL;
	}

	for (i = 0; i < key_list->size; ++i) {
		sensor_name = (char *)pilist_get_data(key_list, i);
		sensor = (struct t_slave_device *)hashtable_get(
			host_device->slave_devices_lookup, sensor_name);

		pilist_add_pointer(sensor_list, sensor);
	}

	return sensor_list;
}
