#include <pcf8574.h>
#include "pilab-i2c-device-strategies.h"
/*
 * CONCRETE I2C DEVICE MODULE LOADING STRATEGIES.
 *
 */

/*
 * Callback for loading the PCF8574 device module.
 */

void i2c_device_pcf8574_loading_strategy_cb(struct t_i2c_device *device,
					    int pin_base, int i2c_addr)
{
	if (pcf8574Setup(pin_base, i2c_addr)) {
		device->pin_base = pin_base;
		device->i2c_addr = i2c_addr;
	}
}
