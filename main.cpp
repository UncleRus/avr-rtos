#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "rtos/core.h"
#include "hardware/uart/uart.h"
#include "hardware/console/console.h"
#include "hardware/mtwi/mtwi.h"
#include "hardware/r2rdac/r2rdac.h"
#include "hardware/hd44780/hd44780.h"

//void led_off (rtos::task_t *task)
//{
//}

//void led_on (rtos::task_t *task)
//{
//
//}

static uint16_t val = 0;

void console_tick (rtos::task_t *task)
{
	//console::tick ();
	val = val >= 0x3ff ? 0 : 0x3ff;
	r2rdac::set (val);
	hd44780::goto_xy (1, 0);
	hd44780::puti (val, 16);
	hd44780::puts ("   ");
}

void on_console (const char *command)
{
	//uart0::send_string (command);
	//console::endl ();
}

int main ()
{
	DDRB |= _BV (PB5);
	//uart0::init (UART_BAUD_SELECT (19200));
	//sdac::init ();
	r2rdac::init ();
	hd44780::init ();
	hd44780::goto_xy (1, 0);
	hd44780::puts ("Hello world");
	hd44780::goto_xy (1, 1);
	hd44780::puts ("Line 2");

	/*
	for (uint8_t addr = 1; addr < 128; addr ++)
	{
		uart0::send_int (addr, 16);
		uart0::send_string (" : ");
		uart0::send_string (mtwi::exists (addr) ? "Yes" : "No");
		console::endl ();
		_delay_ms (500);
	}
	*/
	//const uint8_t *buf = mtwi::exec ("s41?p");
	//mtwi::exec ("s40 fa p");
	//uart0::send_int ((uint16_t) (buf), 16);
	//console::endl ();

	//console::set_callback (on_console);
	rtos::add (console_tick, 3000);

	//rtos::add (led_on, 400);
	rtos::start ();
	while (true)
		;
	return 0;
}
