#ifndef _PILAB_SLAVE_DEVICE_H
#define _PILAB_SLAVE_DEVICE_H

/*
 * Interface functions.
 *
 * These functions should be implemented by the slave devices concrete types.
 */

typedef void(t_slave_device_get_address)(const void *instance);
typedef void(t_slave_device_set_pointer)(const void *instance,
					 const char *property, void *pointer);
typedef int(t_slave_device_get_pin_base)(const void *instance);
typedef int(t_slave_device_get_expansion_pin)(const void *instance, int pin);
typedef const char *(t_slave_device_get_name)(const void *instance);
typedef void(t_slave_device_free_device)(const void *instance);
typedef int(t_slave_device_analog_read)(const void *instance, int pin);
typedef void(t_slave_device_analog_write)(const void *instance, int pin,
					  int value);
typedef int(t_slave_device_digital_read)(const void *instance, int pin);
typedef void(t_slave_device_digital_write)(const void *instance, int pin,
					   int value);

/*
 * Callback functions.
 *
 * These functions should be provided as a callback for the concrete
 * implementations of each slave device, as such these have to be set to NULL.
 *
 * NOTE: These functions can be set through the use of
 * t_slave_device_set_pointer, which allows these to be set on run-time.
 */

typedef int(t_slave_device_init_strategy)(void *arg1, void *arg2);

struct t_slave_device {
	/*
	 * A slave device can be one of several devices, this will contain the
	 * concrete instance of the slave device.
	 */
	void *instance;
	/*
	 * Function used for reading the raw sensor data of the device (analog).
	 */
	t_slave_device_analog_read *analog_read;
	/*
	 * Function used for writing to a specific pin of the device (analog).
	 */
	t_slave_device_analog_write *analog_write;
	/*
	 * Function used for reading the raw sensor data of the device (digital).
	 */
	t_slave_device_digital_read *digital_read;
	/*
	 * Function used for writing to a specific pin of the device (digital).
	 */
	t_slave_device_digital_write *digital_write;
	/*
	 * Allows for changing the callbacks  of the concrete devices,
	 * effectively changing the function pointer to point to alternative
	 * logic.
	 */
	t_slave_device_set_pointer *set_pointer;
	/*
	 * Function used for getting the pin base of the slave device.
	 */
	t_slave_device_get_pin_base *get_pinbase;
	/*
	 * Allows for the slave device to get the memory of the concrete structure.
	 */
	t_slave_device_get_address *get_address;
	/*
	 * Calculates the expansion pin.
	 *
	 * This is needed when interacting with a device that is setup as an
	 * extension module (http://wiringpi.com/extensions/writing-your-own/).
	 */
	t_slave_device_get_expansion_pin *get_expansion_pin;
	/*
	 *  Get the device name.
	 */
	t_slave_device_get_name *get_name;
	/*
	 * Free the device.
	 */
	t_slave_device_free_device *free_device;

	/* Callbacks */

	/*
	 * Initialise the device.
	 */
	t_slave_device_init_strategy *callback_init_strategy;
};

#endif
