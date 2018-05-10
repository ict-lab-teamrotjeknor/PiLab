#include "hd44780.h"

struct hd44780* hd44780_4bit_create(int rs, int en, int db[4])
{
	struct hd44780* lcd = (struct hd44780*)malloc(sizeof(struct hd44780));
	if (!lcd)
		return NULL;

	int fd;
	fd = lcdInit(LCD_ROWS, LCD_COLS, 4, rs, en, db[0], db[1], db[2],
		     db[3], 0, 0, 0, 0);

	if(fd == -1) {
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

struct hd44780* hd44780_8bit_create(int rs, int en, int db[8])
{
	struct hd44780* lcd = (struct hd44780*)malloc(sizeof(struct hd44780));
	if (!lcd)
		return NULL;

	int fd;
	fd = lcdInit(LCD_ROWS, LCD_COLS, 8, rs, en, db[0], db[1], db[2],
		     db[3], db[4], db[5], db[6], db[7]);

	if(fd == -1) {
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

void hd44780_assign_i2c_device(struct hd44780* lcd, struct i2c_device* dev)
{
	if (!lcd)
		return;
	lcd->i2c_device = dev;
}

void hd44780_backlight(struct hd44780* lcd, int pin, int ll)
{
	if (!lcd)
		return;

	if (lcd->i2c_device) {
		int ext_pin = i2c_device_ext_pin(lcd->i2c_device, pin);
		pinMode(ext_pin, OUTPUT);
		digitalWrite(ext_pin, ll);
	}
}

void hd44780_backlight_toggle(struct hd44780* lcd)
{
	if (!lcd)
		return;

	lcd->backlight = (lcd->backlight == HIGH ) ? LOW : HIGH;
	hd44780_backlight(lcd, 3, lcd->backlight);
}

void hd44780_clearscreen(struct hd44780* lcd)
{
	if (!lcd)
		return;

	lcdHome(lcd->fd);
	lcdClear(lcd->fd);
	lcd->x = 0;
	lcd->y = 0;
}

void hd44780_relative_write(struct hd44780* lcd, const char* msg)
{
	if (!lcd)
		return;

	int rw = i2c_device_ext_pin(lcd->i2c_device, 0x01);
	int ca = i2c_device_ext_pin(lcd->i2c_device, 0x03);
	pinMode(rw, OUTPUT);
	pinMode(ca, OUTPUT);
	digitalWrite(rw, LOW);
	digitalWrite(ca, HIGH); 

	size_t l = strlen(msg);
	int ya = (int) ((l + lcd->x) / LCD_COLS);
	int xa = (int) ((l + lcd->x) % LCD_COLS);

	/* TODO: Error to log, invalid write operation <07-05-18, Sjors Sparreboom> */

	/* sanity checks */
	if ((lcd->y < 0) || (lcd->y > LCD_ROWS))
		return;
	if (((ya + lcd->y) < 0) || ((lcd->y + ya) > LCD_ROWS))
		return;
	if (((lcd->y + ya) >= LCD_ROWS) && (lcd->x + l) > LCD_COLS)
		return;

	/* safe to move to point */
	lcdPosition(lcd->fd, lcd->x, lcd->y);

	/* cursor moved, update axioms */
	lcd->y += ya;
	lcd->x = xa;

	lcdPuts(lcd->fd, msg);
}

void hd44780_writeline(struct hd44780* lcd, int y, const char* msg)
{
	if (!lcd)
		return;

	int rw = i2c_device_ext_pin(lcd->i2c_device, 0x01);
	int ca = i2c_device_ext_pin(lcd->i2c_device, 0x03);
	pinMode(rw, OUTPUT);
	pinMode(ca, OUTPUT);
	digitalWrite(rw, LOW);
	digitalWrite(ca, HIGH); 

	size_t l = strlen(msg);
	int ya = (int) (l / LCD_COLS);
	int xa = (l % LCD_COLS);

	/* TODO: Error to log, invalid write operation <07-05-18, Sjors Sparreboom> */

	/* sanity checks */
	if ((y < 0) || (y >= LCD_ROWS))
		return;
	if (((ya + y) < 0) || ((y + ya) > LCD_ROWS))
		return;
	if (((y + ya) >= LCD_ROWS) && (xa + l) > LCD_COLS)
		return;

	/* safe to move to point */
	lcdPosition(lcd->fd, 0, y);

	/* cursor moved, update axioms */
	lcd->y = ya + y;
	lcd->x = xa;

	lcdPuts(lcd->fd, msg);
}

void hd44780_free(struct hd44780* lcd)
{
	if (!lcd)
		return;

	if (lcd->i2c_device)
		i2c_device_free(lcd->i2c_device);
	free(lcd);
}

