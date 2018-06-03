#ifndef _PILAB_I2C_DEVICE_H
#define _PILAB_I2C_DEVICE_H
/*
 * MODULE USED FOR INTERFACING WITH I2C DEVICES.
 */

struct t_i2c_device;

typedef void(t_i2c_device_loading_strategy)(struct t_i2c_device *device,
					    int pin_base, int i2c_addr);

struct t_i2c_device {
	int pin_base;
	int i2c_addr;
	t_i2c_device_loading_strategy *loading_strategy;
};

extern void i2c_device_register(struct t_i2c_device *device);
extern void i2c_device_deregister(struct t_i2c_device *device);

extern struct t_i2c_device *i2c_device_create_with_strategy(
	t_i2c_device_loading_strategy *loading_strategy);
extern struct t_i2c_device *i2c_device_create();
extern void i2c_device_set_loading_strategy(
	struct t_i2c_device *device,
	t_i2c_device_loading_strategy *callback_loading_stategy);
extern void i2c_device_free(struct t_i2c_device *device);
extern int i2c_device_ext_pin(struct t_i2c_device *device, int pin);

#endif
