#ifndef _PILAB_LCD_H
#define _PILAB_LCD_H
#include "pilab-slave-device.h"

/*
 * WHEN CONTROLLING THE DEVICE THROUGH A SPI OR I2C DEVICE WITH A POTENTIOMETER,
 * MAKE SURE IT IS FULLY OPENED, OTHERWISE NO TEXT WILL APPEAR ON THE SCREEN!
 */

struct t_lcd;

typedef void(t_lcd_free_expander_chip)(struct t_lcd *lcd);

enum t_backlight_state {
	LCD_OFF = 0,
	LCD_ON,
};

struct t_lcd_cursor {
	/*
	 * X-axis position of the cursor on the LCD.
	 */
	int x;
	/*
	 * Y-axis position of the cursor on the LCD.
	 */
	int y;
};

struct t_lcd {
	/*
	 * Max amount of rows for the LCD device.
	 */
	int rows;
	/*
	 * Max amount of columns for the LCD device.
	 */
	int columns;
	/*
	 * State of the backlight.
	 *
	 * Defaults to ON.
	 */
	enum t_backlight_state backlight;
	/*
	 * File descriptor for accessing the LCD.
	 */
	int handle;
	/*
	 * Keeps track of the axioms of the LCD display matrix.
	 */
	struct t_lcd_cursor *cursor;
	/*
	 * The LCD can be controlled by an attached expander chip.
	 *
	 * NOTE: Other device interfaces are not yet supported, SPI etc...
	 */
	struct t_slave_device *expander_chip;

	/* Callbacks */

	/*
	 * Callback to free the expander chip.
	 */
	t_lcd_free_expander_chip *callback_free_expander_chip;
};

extern struct t_lcd *lcd_4bit_create(int rows, int columns, int rs, int en,
				     int db[4]);
extern struct t_lcd *lcd_8bit_create(int rows, int columns, int rs, int en,
				     int db[8]);
extern void lcd_assign_expander_chip(struct t_lcd *lcd,
				     struct t_slave_device *expander_chip);
extern void lcd_backlight(struct t_lcd *lcd, int pin,
			  enum t_backlight_state state);
extern void lcd_backlight_toggle(struct t_lcd *lcd);
extern void hd44780_clearscreen(struct t_lcd *lcd);
extern void lcd_relative_write(struct t_lcd *lcd, const char *string);
extern void lcd_writeline(struct t_lcd *lcd, int y, const char *string);
extern void lcd_set_pointer(struct t_lcd *lcd, const char *property,
			    void *pointer);
extern void lcd_free_expander_chip(struct t_lcd *lcd);
extern void lcd_free(struct t_lcd *lcd);
#endif
