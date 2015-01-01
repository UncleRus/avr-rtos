#include "hd44780.h"
#include <avr/io.h>
#include <util/delay.h>

#include "setup.h"
#include <stdlib.h>

// Линии данных
#if !defined(HD44780_PORT_D4) || !defined(HD44780_DDR_D4) || !defined(HD44780_BIT_D4)
	#warning Undefined HD44780 parameter D4
	#define HD44780_PORT_D4 PORTB
	#define HD44780_DDR_D4 DDRB
	#define HD44780_BIT_D4 PB0
#endif
#if !defined(HD44780_PORT_D5) || !defined(HD44780_DDR_D5) || !defined(HD44780_BIT_D5)
	#warning Undefined HD44780 parameter D5
	#define HD44780_PORT_D5 PORTB
	#define HD44780_DDR_D5 DDRB
	#define HD44780_BIT_D5 PB1
#endif
#if !defined(HD44780_PORT_D6) || !defined(HD44780_DDR_D6) || !defined(HD44780_BIT_D6)
	#warning Undefined HD44780 parameter D6
	#define HD44780_PORT_D6 PORTB
	#define HD44780_DDR_D6 DDRB
	#define HD44780_BIT_D6 PB2
#endif
#if !defined(HD44780_PORT_D7) || !defined(HD44780_DDR_D7) || !defined(HD44780_BIT_D7)
	#warning Undefined HD44780 parameter D7
	#define HD44780_PORT_D7 PORTB
	#define HD44780_DDR_D7 DDRB
	#define HD44780_BIT_D7 PB3
#endif

// Линия RS
#if (!defined(HD44780_PORT_RS) || !defined(HD44780_DDR_RS) || !defined(HD44780_BIT_RS))
	#warning Undefined HD44780 parameter RS
	#define HD44780_PORT_RS PORTB
	#define HD44780_DDR_RS DDRB
	#define HD44780_BIT_RS PB4
#endif

// Линия E
#if !defined(HD44780_PORT_E) || !defined(HD44780_DDR_E) || !defined(HD44780_BIT_E)
	#warning Undefined HD44780 parameter E
	#define HD44780_PORT_E PORTB
	#define HD44780_DDR_E DDRB
	#define HD44780_BIT_E PB5
#endif

// кол-во строк
#ifndef HD44780_LINES
	#warning Undefined HD44780 parameter LINES
	#define HD44780_LINES 2
#endif

// Внутренняя длина строки
#ifndef HD44780_LINE_LENGTH
	#define HD44780_LINE_LENGTH 0x40
#endif

#ifndef HD44780_START_LINE1
	#define HD44780_START_LINE1 0x00
#endif
#ifndef HD44780_START_LINE2
	#define HD44780_START_LINE2 0x40
#endif
#ifndef HD44780_START_LINE3
	#define HD44780_START_LINE3 0x14
#endif
#ifndef HD44780_START_LINE4
	#define HD44780_START_LINE4 0x54
#endif

#define HD44780_DATA_MODE HD44780_PORT_RS |= _BV (HD44780_BIT_RS)
#define HD44780_CMD_MODE HD44780_PORT_RS &= ~_BV (HD44780_BIT_RS)

#if HD44780_LINES == 1
	#define HD44780_FUNCTION_DEFAULT    HD44780_FUNCTION_4BIT_1LINE
#else
	#define HD44780_FUNCTION_DEFAULT    HD44780_FUNCTION_4BIT_2LINES
#endif

#define cbi(var, bit) (var &= ~_BV (bit))
#define sbi(var, bit) (var |= _BV (bit))

namespace hd44780
{

volatile static uint8_t _line = 0;
volatile static uint8_t _col = 0;
volatile static bool mutex = false;

#define acquire_mutex() { while (mutex) {} mutex = true; }
#define release_mutex() { mutex = false; }

void _toggle_e ()
{
	sbi (HD44780_PORT_E, HD44780_BIT_E);
	_delay_us (10);
	cbi (HD44780_PORT_E, HD44780_BIT_E);
}

void _out_low (uint8_t data)
{
	if (data & 0x08) sbi (HD44780_PORT_D7, HD44780_BIT_D7);
	else cbi (HD44780_PORT_D7, HD44780_BIT_D7);
	if (data & 0x04) sbi (HD44780_PORT_D6, HD44780_BIT_D6);
	else cbi (HD44780_PORT_D6, HD44780_BIT_D6);
	if (data & 0x02) sbi (HD44780_PORT_D5, HD44780_BIT_D5);
	else cbi (HD44780_PORT_D5, HD44780_BIT_D5);
	if (data & 0x01) sbi (HD44780_PORT_D4, HD44780_BIT_D4);
	else cbi (HD44780_PORT_D4, HD44780_BIT_D4);
}

void _out_high (uint8_t data)
{
	if (data & 0x80) sbi (HD44780_PORT_D7, HD44780_BIT_D7);
	else cbi (HD44780_PORT_D7, HD44780_BIT_D7);
	if (data & 0x40) sbi (HD44780_PORT_D6, HD44780_BIT_D6);
	else cbi (HD44780_PORT_D6, HD44780_BIT_D6);
	if (data & 0x20) sbi (HD44780_PORT_D5, HD44780_BIT_D5);
	else cbi (HD44780_PORT_D5, HD44780_BIT_D5);
	if (data & 0x10) sbi (HD44780_PORT_D4, HD44780_BIT_D4);
	else cbi (HD44780_PORT_D4, HD44780_BIT_D4);
}

void _write (uint8_t data, bool rs)
{
	sbi (HD44780_DDR_D4, HD44780_BIT_D4);
	sbi (HD44780_DDR_D5, HD44780_BIT_D5);
	sbi (HD44780_DDR_D6, HD44780_BIT_D6);
	sbi (HD44780_DDR_D7, HD44780_BIT_D7);

	_out_high (data);
	if (rs) HD44780_DATA_MODE;
	else HD44780_CMD_MODE;
	_toggle_e ();

	_out_low (data);
	if (rs) HD44780_DATA_MODE;
	else HD44780_CMD_MODE;
	_toggle_e ();
}

void _wait (bool more)
{
	_delay_us (10);
	if (more) _delay_ms (2);
}

void _next_line ()
{
	_col = 0;
	if (++ _line == HD44780_LINES)
		_line = 0;
	goto_xy (_col, _line);
}

void init (uint8_t attribute)
{
	acquire_mutex ();

	sbi (HD44780_DDR_D4, HD44780_BIT_D4);
	sbi (HD44780_DDR_D5, HD44780_BIT_D5);
	sbi (HD44780_DDR_D6, HD44780_BIT_D6);
	sbi (HD44780_DDR_D7, HD44780_BIT_D7);
	sbi (HD44780_DDR_E, HD44780_BIT_E);
	sbi (HD44780_DDR_RS, HD44780_BIT_RS);

	_delay_ms (15);

	_out_high (HD44780_FUNCTION_8BIT_1LINE);
	_toggle_e ();
	_delay_ms (2);
	_out_high (HD44780_FUNCTION_8BIT_1LINE);
	_toggle_e ();
	_delay_ms (2);
	_out_high (HD44780_FUNCTION_8BIT_1LINE);
	_toggle_e ();
	_delay_ms (2);

	_out_high (HD44780_FUNCTION_4BIT_1LINE);
	_toggle_e ();
	command (HD44780_FUNCTION_DEFAULT);
	command (HD44780_DISP_OFF);
	clear ();
	command ((1 << HD44780_ENTRY_MODE) | (1 << HD44780_ENTRY_INC));
	command (attribute);
	_wait (true);

	_line = _col = 0;

	release_mutex ();
}

void command (uint8_t command)
{
	_wait (false);
	_write (command, false);
	_wait (true);
}

void put (char c)
{
	acquire_mutex ();

	_wait (false);
	_write ((uint8_t) c, true);
	_wait (false);
	if (++ _col == HD44780_COLS) _next_line ();

	release_mutex ();
}

void puts (const char *s)
{
	while (*s)
	{
		put (*s);
		s++;
	}
}

void puts_p (const char *progmem_str)
{
	register char c;
	while ((c = pgm_read_byte (progmem_str ++)))
		put (c);
}

inline void clear ()
{
	command (1 << HD44780_CLR);
	_line = _col = 0;
}

void puti (int i, int radix)
{
	char buffer [24];
	itoa (i, buffer, radix);
	puts (buffer);
}

void goto_xy (uint8_t x, uint8_t y)
{
#if HD44780_LINES == 1
	command ((1 << HD44780_DDRAM) + HD44780_START_LINE1 + x);
#elif HD44780_LINES == 2
	if (!y) command ((1 << HD44780_DDRAM) + HD44780_START_LINE1 + x);
	else command ((1 << HD44780_DDRAM) + HD44780_START_LINE2 + x);
#elif HD44780_LINES == 3
	if (!y) command ((1 << HD44780_DDRAM) + HD44780_START_LINE1 + x);
	else if (y == 1) command ((1 << HD44780_DDRAM) + HD44780_START_LINE2 + x);
	else command ((1 << HD44780_DDRAM) + HD44780_START_LINE3 + x);
#elif HD44780_LINES == 4
	if (!y) command ((1 << HD44780_DDRAM) + HD44780_START_LINE1 + x);
	else if (y == 1) command ((1 << HD44780_DDRAM) + HD44780_START_LINE2 + x);
	else if (y == 2) command ((1 << HD44780_DDRAM) + HD44780_START_LINE3 + x);
	else command ((1 << HD44780_DDRAM) + HD44780_START_LINE4 + x);
#endif
	_line = y;
	_col = x;
}

}
