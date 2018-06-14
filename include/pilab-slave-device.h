#ifndef _PILAB_SLAVE_DEVICE_H
#define _PILAB_SLAVE_DEVICE_H

/*
 * Interface functions.
 *
 * These functions should be implemented by the slave devices concrete types.
 */

typedef void(t_slave_device_init)(const void *instance);
typedef void(t_slave_device_set_pointer)(const void *instance);
typedef int(t_slave_device_get_expansion_pin)(const void *instance, int pin);
typedef void(t_slave_device_close_handle)(const void *instance);

/*
 * Callback functions.
 *
 * These functions should be provided as a callback for the concrete
 * implementations of each slave device.
 *
 * NOTE: These functions can be set through the use of
 * t_slave_device_set_pointer, which allows these to be set on run-time.
 */

typedef int(t_slave_device_read)(const void *instance, int pin);
typedef void(t_slave_device_write)(const void *instance, int pin, int value);

struct t_slave_device {
	/*
	 * A slave device can be one of several devices, this will contain the
	 * concrete instance of the slave device.
	 */
	void *instance;
	/*
	 * Function used for reading the raw sensor data of the device.
	 */
	t_slave_device_read *read;
	/*
	 * Function used for writing to a specific pin of the device.
	 */
	t_slave_device_write *write;
	/*
	 * Initialise the device.
	 */
	t_slave_device_init *init;
	/*
	 * Allows for changing the callbacks  of the concrete devices,
	 * effectively changing the function pointer to point to alternative
	 * logic.
	 */
	t_slave_device_set_pointer *set_pointer;
	/*
	 * Calculates the expansion pin.
	 *
	 * This is needed when interacting with a device that is setup as an
	 * extension module (http://wiringpi.com/extensions/writing-your-own/).
	 */
	t_slave_device_get_expansion_pin *get_expansion_pin;
	/*
	 * Closes the handle.
	 *
	 * NOTE: Defaults to NULL, meaning we keep the handle, which is handy,
	 * as this will improve access time a little.
	 */
	t_slave_device_close_handle *close_handle;
};

#endif
