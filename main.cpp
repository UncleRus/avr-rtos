#include <avr/io.h>
#include <avr/interrupt.h>

#include "rtos/core.h"
#include "hardware/uart/uart.h"
#include "hardware/console/console.h"

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
	//DDRB |= _BV (PB5);
	DDRB = 0xff;
	uart0::init (UART_BAUD_SELECT (19200));

	console::set_callback (on_console);
	rtos::add (console_tick, 50);

	rtos::add (led_on, 400);
	rtos::start ();
	while (true)
		;
	return 0;
}
