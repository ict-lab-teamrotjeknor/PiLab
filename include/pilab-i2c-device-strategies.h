#ifndef _PILAB_I2C_DEVICE_STRATEGIES_H
#define _PILAB_I2C_DEVICE_STRATEGIES_H
#include "pilab-i2c-device.h"

/*
 * INTERFACES TO THE CONCRETE I2C DEVICE LOADING STRATEGIES.
 */

/*
 * Interface to the PCF8574 loading strategy.
 */

void i2c_device_pcf8574_loading_strategy(struct t_i2c_device *device,
					 int pin_base, int i2c_addr);

#endif
