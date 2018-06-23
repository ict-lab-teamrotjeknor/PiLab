#ifndef _PILAB_HOST_DEVICE_H
#define _PILAB_HOST_DEVICE_H
#include "pilab-hashtable.h"
#include "pilab-slave-device.h"

enum t_host_device_sensor_types {
	HOST_DEVICE_GPIO = 0,
	HOST_DEVICE_I2C,
	HOST_DEVICE_LCD_I2C,
	/*
	 * Number of fields.
	 */
	HOST_DEVICE_NUM_TYPES,
};

enum t_host_device_sensor_modules {
	HOST_DEVICE_MODULE_DS18B20 = 0,
	HOST_DEVICE_MODULE_PCF8574,
	HOST_DEVICE_MODULE_HD44780,
	/*
	 * Number of fields.
	 */
	HOST_DEVICE_NUM_MODULES,
};

struct t_host_device {
	/*
	 * Hashtable with the slaves.
	 */
	struct t_hashtable *slave_devices_lookup;
	/*
	 * For now a host device, will have an lcd that is not a slave_device.
	 */
	struct t_lcd *lcd;
};

/* Strings for the sensor types */
#define PILAB_HOST_DEVICE_GPIO "gpio"
#define PILAB_HOST_DEVICE_I2C "i2c"
#define PILAB_HOST_DEVICE_LCD_I2C "lcd_i2c"

/* Strings for the module types */
#define PILAB_HOST_MODULE_DS18B20 "ds18b20"
#define PILAB_HOST_MODULE_PCF8574 "pcf8574"
#define PILAB_HOST_MODULE_HD44780 "hd44780"

extern int host_device_get_sensor_module(const char *type);
extern int host_device_get_sensor_type(const char *type);
extern struct t_host_device *host_device_create(void);
extern void host_device_free_device_default_cb(struct t_hashtable *hashtable,
					       const void *key, void *value);
extern void host_device_free_name_default_cb(struct t_hashtable *hashtable,
					     void *key);
extern void host_device_module_register(struct t_host_device *host_device,
					char *sensor_name, int pin_base,
					char *addr);

extern int host_device_slave_builder(struct t_host_device *host_device,
				     struct t_pilist *slave_components);
extern struct t_host_device *
	host_device_create_with_max_registrations_size(int size);
extern struct t_host_device *host_device_create(void);
extern void host_device_register_slave_device(
	struct t_host_device *host_device,
	const struct t_slave_device *slave_device);
extern void host_device_deregister_slave_device(
	struct t_host_device *host_device,
	const struct t_slave_device *slave_device);
extern void host_device_deregister_all_slave_devices(
	struct t_host_device *host_device);
extern int
	host_device_get_slave_devices_count(struct t_host_device *host_device);
extern void
	host_device_read_in_sensor_modules(struct t_host_device *host_device);
extern struct t_pilist *
	host_device_get_sensor_name_list(struct t_host_device *host_device);
extern struct t_pilist *
	host_device_get_sensor_list(struct t_host_device *host_device);

#endif
