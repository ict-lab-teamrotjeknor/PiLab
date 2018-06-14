#include <stdlib.h>
#include <wiringPi.h>
#include <lcd.h>
#include <string.h>
#include "pilab-string.h"
#include "pilab-lcd.h"

/*
 * Create a new LCD using a 4-bit interface
 *
 * Returns a pointer to the 4-bit LCD display, NULL otherwise.
 */

struct t_lcd *lcd_4bit_create(int rows, int columns, int rs, int en, int db[4])
{
	struct t_lcd *lcd;
	struct t_lcd_cursor *cursor;
	int fd;

	lcd = malloc(sizeof(*lcd));
	cursor = malloc(sizeof(*cursor));

	if (!lcd || cursor)
		return NULL;

	fd = lcdInit(rows, columns, 4, rs, en, db[0], db[1], db[2], db[3], 0, 0,
		     0, 0);

	if (fd == -1) {
		/* TODO: Log event: error with initializing lcd  <06-05-18, Sjors Sparreboom> */
		free(lcd);
		return NULL;
	}

	cursor->x = 0;
	cursor->y = cursor->x;

	lcd->backlight = LCD_ON;
	lcd->handle = fd;
	lcd->rows = rows;
	lcd->columns = columns;
	lcd->cursor = cursor;
	lcd->expander_chip = NULL;
	lcd->callback_free_expander_chip = NULL;

	return lcd;
}

/*
 * Create a new LCD using a 8-bit interface
 *
 * Returns a pointer to the 8-bit LCD display, NULL otherwise.
 */

struct t_lcd *lcd_8bit_create(int rows, int columns, int rs, int en, int db[8])
{
	struct t_lcd *lcd;
	struct t_lcd_cursor *cursor;
	int fd;

	lcd = malloc(sizeof(*lcd));
	cursor = malloc(sizeof(*cursor));

	if (!lcd || cursor)
		return NULL;

	fd = lcdInit(rows, columns, 8, rs, en, db[0], db[1], db[2], db[3],
		     db[4], db[5], db[6], db[7]);

	if (fd == -1) {
		/* TODO: Log event: error with initializing lcd  <06-05-18, Sjors Sparreboom> */
		free(lcd);
		return NULL;
	}

	cursor->x = 0;
	cursor->y = cursor->x;

	lcd->backlight = LCD_ON;
	lcd->handle = fd;
	lcd->rows = rows;
	lcd->columns = columns;
	lcd->cursor = cursor;
	lcd->expander_chip = NULL;
	lcd->callback_free_expander_chip = NULL;

	return lcd;
}

/*
 * If interfacing with the LCD through a expander chip.
 *
 * NOTE: This is currently the supported and recommended way of interfacing
 * with the LCD.
 */

void lcd_assign_expander_chip(struct t_lcd *lcd,
			      struct t_slave_device *expander_chip)
{
	if (!lcd)
		return;

	lcd->expander_chip = expander_chip;
}

/*
 * Switch off the backlight using the specified logic level.
 */

void lcd_backlight(struct t_lcd *lcd, int pin, enum t_backlight_state state)
{
	int ext_pin;

	if (!lcd)
		return;

	ext_pin = pin;
	if (lcd->expander_chip)
		ext_pin = lcd->expander_chip->callback_get_expansion_pin(
			lcd->expander_chip->instance, pin);

	pinMode(ext_pin, OUTPUT);
	digitalWrite(ext_pin, state);
}

/*
 * Toggle the backlight of the LCD, turning it on or off respectively.
 */

void lcd_backlight_toggle(struct t_lcd *lcd)
{
	if (!lcd)
		return;

	lcd->backlight = (lcd->backlight == LCD_ON) ? LCD_OFF : LCD_ON;
	lcd_backlight(lcd, 3, lcd->backlight);
}

/*
 * Erase the written content from the LCD.
 */

void hd44780_clearscreen(struct t_lcd *lcd)
{
	if (!lcd)
		return;

	lcdHome(lcd->handle);
	lcdClear(lcd->handle);
	lcd->cursor->x = 0;
	lcd->cursor->y = lcd->cursor->x;
}

/*
 * Write text relative already written text, effectively append text to the old
 * text.
 *
 * Keeps track of the ending position, by manually moving the cursor to the x/y
 * axioms of the crystallised matrix display.
 */

void lcd_relative_write(struct t_lcd *lcd, const char *string)
{
	/* ca */
	int rw, ya, xa;
	size_t len;

	if (!lcd)
		return;

	rw = lcd->expander_chip->callback_get_expansion_pin(
		lcd->expander_chip->instance, 0x01);
	/* ca = lcd->expander_chip->callback_get_expansion_pin( */
	/* 	lcd->expander_chip->instance, 0x03); */

	pinMode(rw, OUTPUT);

	digitalWrite(rw, LOW);
	/* digitalWrite(ca, HIGH); */

	len = strlen(string);
	ya = ((len + lcd->cursor->x) / lcd->columns);
	xa = ((len + lcd->cursor->x) % lcd->columns);

	/* TODO: Error to log, invalid write operation <07-05-18, Sjors Sparreboom> */

	/* sanity checks */
	if ((lcd->cursor->y < 0) || (lcd->cursor->y > lcd->rows))
		return;
	if (((ya + lcd->cursor->y) < 0) || ((lcd->cursor->y + ya) > lcd->rows))
		return;
	if (((lcd->cursor->y + ya) >= lcd->rows) &&
	    (int)(lcd->cursor->x + len) > lcd->columns)
		return;

	/* safe to move to point */
	lcdPosition(lcd->handle, lcd->cursor->x, lcd->cursor->y);

	/* cursor moved, update axioms */
	lcd->cursor->y += ya;
	lcd->cursor->x = xa;

	lcdPuts(lcd->handle, string);
}

/*
 * Write a line of the y axis, this will overwrite any other placed text on this
 * line.
 */

void lcd_writeline(struct t_lcd *lcd, int y, const char *string)
{
	/* ca */
	int rw, ya, xa;
	size_t len;

	if (!lcd)
		return;
	rw = lcd->expander_chip->callback_get_expansion_pin(
		lcd->expander_chip->instance, 0x01);
	/* ca = lcd->expander_chip->callback_get_expansion_pin( */
	/* 	lcd->expander_chip->instance, 0x03); */

	pinMode(rw, OUTPUT);

	digitalWrite(rw, LOW);
	/* digitalWrite(ca, HIGH); */

	len = strlen(string);
	ya = (int)(len / lcd->columns);
	xa = (len % lcd->columns);

	/* TODO: Error to log, invalid write operation <07-05-18, Sjors Sparreboom> */

	/* sanity checks */
	if ((y < 0) || (y >= lcd->rows))
		return;
	if (((ya + y) < 0) || ((y + ya) > lcd->rows))
		return;
	if (((y + ya) >= lcd->rows) && (int)(xa + len) > lcd->columns)
		return;

	/* safe to move to point */
	lcdPosition(lcd->handle, 0, y);

	/* cursor moved, update axioms */
	lcd->cursor->y = ya + y;

	lcdPuts(lcd->handle, string);
}

/*
 * Set LCD property (pointer)
 *
 * NOTE: Currently the properties that are allowed to be set:
 * - callback_free_expander_chip
 */

void lcd_set_pointer(struct t_lcd *lcd, const char *property, void *pointer)
{
	if (!lcd || !property)
		return;

	if (string_strcasecmp(property, "callback_free_expander_chip"))
		lcd->callback_free_expander_chip = pointer;
}

/*
 * Free the space used by the expander chip.
 */

void lcd_free_expander_chip(struct t_lcd *lcd)
{
	if (!lcd)
		return;

	if (lcd->callback_free_expander_chip)
		(void)(lcd->callback_free_expander_chip)(lcd);
}

/*
 * Free the lcd.
 */

void lcd_free(struct t_lcd *lcd)
{
	if (!lcd)
		return;

	/* free attached expander chip */
	lcd_free_expander_chip(lcd);

	/* free the lcd cursor */
	free(lcd->cursor);
	/* free the lcd */
	free(lcd);
}
