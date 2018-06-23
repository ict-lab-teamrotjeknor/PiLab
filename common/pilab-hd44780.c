#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include <lcd.h>
#include "pilab-hd44780.h"

/*
 * Create a new hd44780 device using the 4bit interface
 *
 * Returns a pointer to the new device, NULL otherwise.
 */

struct hd44780 *hd44780_4bit_create(int rs, int en, int db[4])
{
	struct hd44780 *lcd;
	int fd;

	lcd = malloc(sizeof(*lcd));

	if (!lcd)
		return NULL;

	fd = lcdInit(LCD_ROWS, LCD_COLS, 4, rs, en, db[0], db[1], db[2], db[3],
		     0, 0, 0, 0);

	if (fd == -1) {
		/* TODO: Log event: error with initializing lcd  <06-05-18, Sjors Sparreboom> */
		hd44780_free(lcd);
		return NULL;
	}

	lcd->backlight = HIGH;
	lcd->fd = fd;
	lcd->y = 0;
	lcd->x = 0;

	return lcd;
}

/*
 * Create a new hd44780 device using the 8bit interface
 *
 * Returns a pointer to the new device, NULL otherwise.
 */

struct hd44780 *hd44780_8bit_create(int rs, int en, int db[8])
{
	struct hd44780 *lcd;
	int fd;

	lcd = malloc(sizeof(*lcd));
	if (!lcd)
		return NULL;

	fd = lcdInit(LCD_ROWS, LCD_COLS, 8, rs, en, db[0], db[1], db[2], db[3],
		     db[4], db[5], db[6], db[7]);

	if (fd == -1) {
		/* TODO: Log event: error with initializing lcd  <06-05-18, Sjors Sparreboom> */
		hd44780_free(lcd);
		return NULL;
	}

	lcd->backlight = HIGH;
	lcd->fd = fd;
	lcd->y = 0;
	lcd->x = 0;

	return lcd;
}

/*
 * When controlled by an I2C device call this method.
 *
 * NOTE: Currently this is the only way of interfacing with the device.
 */

void hd44780_assign_i2c_device(struct hd44780 *lcd, struct t_i2c_device *dev)
{
	if (!lcd)
		return;

	lcd->i2c_device = dev;
}

/*
 * Control the back-light by letting you set the logic level for the back-light pin.
 */

void hd44780_backlight(struct hd44780 *lcd, int pin, int ll)
{
	int ext_pin;

	if (!lcd)
		return;

	if (lcd->i2c_device) {
		ext_pin = i2c_device_get_expansion_pin(lcd->i2c_device, pin);
		pinMode(ext_pin, OUTPUT);
		digitalWrite(ext_pin, ll);
	}
}

/*
 * Toggle the back-light of the HD44780 device, turning it on or off respectively.
 */

void hd44780_backlight_toggle(struct hd44780 *lcd)
{
	if (!lcd)
		return;

	lcd->backlight = (lcd->backlight == HIGH) ? LOW : HIGH;
	hd44780_backlight(lcd, 3, lcd->backlight);
}

/*
 * Erase the written content from the screen.
 */

void hd44780_clearscreen(struct hd44780 *lcd)
{
	if (!lcd)
		return;

	lcdHome(lcd->fd);
	lcdClear(lcd->fd);
	lcd->x = 0;
	lcd->y = 0;
}

/*
 * Write text relative already written text, effectively append text to the old
 * text.
 *
 * Keeps track of the ending position, by manually moving the cursor to the x/y
 * axioms of the crystallised matrix display.
 */

void hd44780_relative_write(struct hd44780 *lcd, const char *msg)
{
	int rw, ca, ya, xa;
	size_t len;

	if (!lcd)
		return;

	rw = i2c_device_get_expansion_pin(lcd->i2c_device, 0x01);
	ca = i2c_device_get_expansion_pin(lcd->i2c_device, 0x03);
	pinMode(rw, OUTPUT);

	digitalWrite(rw, LOW);
	digitalWrite(ca, HIGH);

	len = strlen(msg);
	ya = (int)((len + lcd->x) / LCD_COLS);
	xa = (int)((len + lcd->x) % LCD_COLS);

	/* TODO: Error to log, invalid write operation <07-05-18, Sjors Sparreboom> */

	/* sanity checks */
	if ((lcd->y < 0) || (lcd->y > LCD_ROWS))
		return;
	if (((ya + lcd->y) < 0) || ((lcd->y + ya) > LCD_ROWS))
		return;
	if (((lcd->y + ya) >= LCD_ROWS) && (lcd->x + len) > LCD_COLS)
		return;

	/* safe to move to point */
	lcdPosition(lcd->fd, lcd->x, lcd->y);

	/* cursor moved, update axioms */
	lcd->y += ya;
	lcd->x = xa;

	lcdPuts(lcd->fd, msg);
}

/*
 * Write a line of the y axis, this will overwrite any other placed text on this
 * line.
 */

void hd44780_writeline(struct hd44780 *lcd, int y, const char *msg)
{
	int rw, ca, ya, xa;
	size_t len;

	if (!lcd)
		return;

	rw = i2c_device_get_expansion_pin(lcd->i2c_device, 0x01);
	ca = i2c_device_get_expansion_pin(lcd->i2c_device, 0x03);

	pinMode(rw, OUTPUT);
	pinMode(ca, OUTPUT);
	digitalWrite(rw, LOW);
	digitalWrite(ca, HIGH);

	len = strlen(msg);
	ya = (int)(len / LCD_COLS);
	xa = (len % LCD_COLS);

	/* TODO: Error to log, invalid write operation <07-05-18, Sjors Sparreboom> */

	/* sanity checks */
	if ((y < 0) || (y >= LCD_ROWS))
		return;
	if (((ya + y) < 0) || ((y + ya) > LCD_ROWS))
		return;
	if (((y + ya) >= LCD_ROWS) && (xa + len) > LCD_COLS)
		return;

	/* safe to move to point */
	lcdPosition(lcd->fd, 0, y);

	/* cursor moved, update axioms */
	lcd->y = ya + y;
	lcd->x = xa;

	lcdPuts(lcd->fd, msg);
}

/*
 * Everything that is born, must perish.
 */

void hd44780_free(struct hd44780 *lcd)
{
	if (!lcd)
		return;

	/* free any attached I2C devices */
	if (lcd->i2c_device)
		i2c_device_free_device(lcd->i2c_device);

	free(lcd);
}
