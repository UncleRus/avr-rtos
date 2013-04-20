#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "rtos/core.h"
#include "hardware/uart/uart.h"
#include "hardware/console/console.h"
#include "hardware/mtwi/mtwi.h"

void led_off (rtos::task_t *task)
{
	PORTB &= ~_BV (PB5);
}

void led_on (rtos::task_t *task)
{
	PORTB |= _BV (PB5);
	rtos::add (led_off, 200, 1);
}

void console_tick (rtos::task_t *task)
{
	console::tick ();
}

void on_console (const char *command)
{
	uart0::send_string (command);
	console::endl ();
}

int main ()
{
	DDRB |= _BV (PB5);
	uart0::init (UART_BAUD_SELECT (19200));

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
	mtwi::exec ("s40 fa p");
	//uart0::send_int ((uint16_t) (buf), 16);
	//console::endl ();

	console::set_callback (on_console);
	rtos::add (console_tick, 50);

	rtos::add (led_on, 400);
	rtos::start ();
	while (true)
		;
	return 0;
}
