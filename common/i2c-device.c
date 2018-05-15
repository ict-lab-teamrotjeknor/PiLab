#include "i2c-device.h"

struct i2c_device* i2c_device_create(enum i2c_type type, int pin_base, int i2c_addr)
{
	/* TODO: Event error: Log unsupported i2c_device <07-05-18, Sjors Sparreboom> */
	switch (type) {
	case PCF8574:
		break;
	default:
		printf("%s\n", "Unsupported i2c_device");
		break;
	}
	struct i2c_device* dev = (struct i2c_device*)malloc(sizeof(struct i2c_device));
	if (!dev)
		return NULL;

	/* TODO: Refactor to own header <14-05-18, Sjors Sparreboom> */
	if (pcf8574Setup(pin_base, i2c_addr)) {
		dev->i2c_addr = i2c_addr;
		dev->pin_base = pin_base;
	}
}

void i2c_device_free(struct i2c_device* dev)
{
	if (!dev)
		return;
	free(dev);
}

int i2c_device_ext_pin(struct i2c_device* dev, int pin)
{
	/* TODO: Log event: pin error device not found <06-05-18, Sjors Sparreboom> */
	if (!dev)
		return -1;
	return (dev->pin_base + pin);
}
