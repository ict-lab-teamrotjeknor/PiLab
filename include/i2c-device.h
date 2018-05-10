#ifndef _PILAB_I2C_DEVICE_H
#define _PILAB_I2C_DEVICE_H
#include <wiringPi.h>

/*! \enum i2c_type
 *
 *  Types of i2c_type currently supported
 */
enum i2c_type { PCF8574 };

struct i2c_device {
	enum i2c_type type;
	int pin_base;
	int i2c_addr;
};

struct i2c_device* i2c_device_create(enum i2c_type type, int pin_base, int i2c_addr);

void i2c_device_free(struct i2c_device* dev);
int i2c_device_ext_pin(struct i2c_device* dev, int pin);

#endif _PILAB_I2C_DEVICE_H
