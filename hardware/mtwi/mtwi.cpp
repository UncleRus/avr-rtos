#include "mtwi.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "rtos/core.h"

#include "setup.h"

#ifndef MTWI_TIMEOUT
#	define MTWI_TIMEOUT 500
#endif

#ifndef MTWI_RX_BUFFER_SIZE
#	define MTWI_RX_BUFFER_SIZE 16
#endif

namespace mtwi
{

volatile static bool mutex = false;

#define acquire_mutex() { while (mutex) {} mutex = true; }
#define release_mutex() { mutex = false; }

volatile static bool timeout = false;

#define send_start()		{ TWCR = _BV (TWINT) | _BV (TWSTA) | _BV (TWEN); }
#define is_start_sent		((TWSR & 0xf8) == 0x08 || (TWSR & 0xf8) == 0x10)
#define send_stop()			{ TWCR = _BV (TWINT) | _BV (TWSTO) | _BV (TWEN); }
#define send_byte(b) 		{ TWDR = b; TWCR = _BV (TWINT) | _BV (TWEN); }

#define ask_byte()			{ TWCR = _BV (TWINT) | _BV (TWEN) | _BV (TWEA); }
#define ask_last_byte()		{ TWCR = _BV (TWINT) | _BV (TWEN); }
#define is_byte_received	((TWSR & 0xf8) == 0x50 || (TWSR & 0xf8) == 0x58)
#define is_byte_acked		((TWSR & 0xf8) == 0x18 || (TWSR & 0xf8) == 0x40 || (TWSR & 0xf8) == 0x28 || (TWSR & 0xf8) == 0x50)


void _on_timeout (rtos::task_t *task)
{
	timeout = true;
}

const char _rx_buffer [MTWI_RX_BUFFER_SIZE];
const char *_rx_end = _rx_buffer + MTWI_RX_BUFFER_SIZE - 1;
const char *_rx_position = _rx_buffer;

uint8_t utils::hex2nibble (char hex)
{
	hex -= '0';
	if (hex < 0) return 0;
	if (hex < 10) return hex;
	hex -= 7;
	if (hex < 10) return 0;
	if (hex < 16) return hex;
	hex -= 32;
	return hex > 9 && hex < 16 ? hex : 0;
}

void _wait ()
{
	rtos::task_t *timeout_task = rtos::add (_on_timeout, MTWI_TIMEOUT, 1);
	while (!(TWCR & _BV (TWINT)) && !timeout)
		;
	if (timeout) error = MTWI_ERR_TIMEOUT;
		else rtos::remove (timeout_task);
}

uint8_t _exec (char cmd, char arg)
{
	if (!cmd) return 0;
	switch (cmd)
	{
		case 's':
			// start, repeated start
			send_start ();
			_wait ();
			if (error) return 0;
			if (!is_start_sent)
			{
				error = MTWI_ERR_BUS_FAIL;
				return 0;
			}
			return 1;
		case 'p':
			// stop
			send_stop ();
			return 1;
		case ' ':
			// nothing to do
			return 1;
		case '.':
			// delay 10 ms
			_delay_ms (10);
			return 1;
		case '-':
			// delay 100 ms
			_delay_ms (100);
			return 1;
		case 'S':
			// set speed
			switch (arg)
			{
				case 'H':
					// high speed - 400 kHz
					TWBR = 0x02;
					break;
				case 'L':
					// low speed - 15 kHz
					TWBR = 0xff;
					break;
				default:
					// default speed - 100 kHz
					TWBR = 0x20;
					break;
			}
			return 2;
		case '?':
			// ask byte
			if (_rx_position == _rx_end)
			{
				error = MTWI_ERR_BUFFER_OVERFLOW;
				return 0;
			}
			if (arg == '?') ask_byte ()
				else ask_last_byte ();
			_wait ();
			if (error) return 0;
			if (!is_byte_received)
			{
				error = MTWI_ERR_BUSY;
				return 0;
			}
			*(++ _rx_position) = TWDR;
			return 1;
	}
	// send byte by default
	send_byte (
		arg
			? (utils::hex2nibble (cmd) << 4) | utils::hex2nibble (arg)
			: utils::hex2nibble (cmd)
	);
	_wait ();
	if (error) return 0;
	if (!is_byte_acked ())
	{
		error = MTWI_ERR_NOT_ACKED;
		return 0;
	}
	return 2;
}

const char *exec (const char *cmd)
{
	acquire_mutex ();

	error = 0;
	while (*cmd)
	{
		uint8_t offset = _exec (*cmd, *(cmd + 1));
		if (!offset) break;
		cmd += offset;
	}

	release_mutex ();
	return _rx_buffer;
}

const char *exec_p (const char *progmem_cmd)
{
	acquire_mutex ();

	error = 0;
	do
	{
		char cmd = pgm_read_byte (progmem_cmd);
		if (!cmd) break;
		uint8_t offset = _exec (cmd, pgm_read_byte (progmem_cmd + 1));
		progmem_cmd += offset;
	}
	while (!error);

	release_mutex ();
	return _rx_buffer;
}

}

