#ifndef _PILAB_GPIO_DEVICE_H
#define _PILAB_GPIO_DEVICE_H
#include "pilab-slave-device.h"
#include "pilab-host-device.h"

typedef int(t_gpio_device_init_strategy)(void *arg1, void *arg2);

struct t_gpio_device {
	/*
	 * The name of the device.
	 */
	const char *device_name;
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

	/* Callbacks */

	/*
	 * Callback used for loading the specific device module.
	 *
	 * NOTE: Upon initialisation of your concrete type, this should be set
	 * to NULL as implementation details might vary so much that a good
	 * default isn't possible.
	 */
	t_gpio_device_init_strategy *callback_init_strategy;
};

extern int gpio_device_analog_read(const void *instance, int pin);
extern void gpio_device_analog_write(const void *instance, int pin, int value);
extern int gpio_device_digitial_read(const void *instance, int pin);
extern void gpio_device_digital_write(const void *instance, int pin, int value);
extern void gpio_device_init(const void *instance);
extern int gpio_device_get_pin_base(const void *instance);
extern void gpio_device_set_pointer(const void *instance, const char *property,
				    void *pointer);
extern int gpio_device_get_expansion_pin(const void *instance, int pin);
extern int gpio_device_get_address(const void *instance);
extern const char *gpio_device_get_name(const void *instance);
extern void gpio_device_free_device(const void *instance);
extern struct t_gpio_device *gpio_device_create(int pin_base, const char *device_name,
						struct t_host_device *host);
#endif
