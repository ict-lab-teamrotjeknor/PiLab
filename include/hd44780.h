#ifndef _PILAB_HD44780_H
#define _PILAB_HD44780_H
#include "i2c-device.h"
#include <wiringPi.h>

/*
 * PLEASE MAKE SURE THE POTENTIOMETER IS FULLY OPENED, OTHERWISE NO TEXT WILL
 * APPEAR ON THE SCREEN
 *
 */

/*
 * MAPPING: PCF8574T <-> HD44780
 *
 * P0 <—> RS
 * P1 <—> R/W
 * P2 <—> EN
 * P3 <—> CATHODE+
 * P4 <—> DB4
 * P5 <—> DB5
 * P6 <—> DB6
 * P7 <—> DB7
 *
 * TRANSFER 4-BIT BUSLINE: (DB4~DB7)
 *
 * TRANSFER 8-BIT BUSLINE: (DB7~DB4) -> (DB3~DB0)
 *
 */

/* DISPLAY SETUP */
#define LCD_ROWS 4
#define LCD_COLS 20

struct hd44780 {
	int y;
	int x;
	int backlight;
	int fd;
	struct i2c_device* i2c_device;
};


struct hd44780* hd44780_4bit_create(int rs, int en, int db[4]);
struct hd44780* hd44780_8bit_create(int rs, int en, int db[8]);

void hd44780_backlight(struct hd44780* lcd, int pin, int ll);
void hd4470_backlight_toggle(struct hd44780* lcd);
void hd44780_assign_i2c_device(struct hd44780* lcd, struct i2c_device* dev);
void hd44780_relative_write(struct hd44780* lcd, const char* msg);
void hd44780_writeline(struct hd44780* lcd, int y, const char* msg);
void hd44780_clearscreen(struct hd44780* lcd);
void hd44780_free(struct hd44780* lcd);

#endif _PILAB_HD44780_H

