#include "console.h"
#include <stddef.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include "hardware/uart/uart.h"

#define KEY_ENTER	0x0D
#define KEY_BS		0x08
#define KEY_DEL		0x7F
#define KEY_ESC		0x1B

#ifndef DECLARE_PGM_STR
#define DECLARE_PGM_STR(var, value) extern const char PROGMEM var []; const char var [] = value;
#endif

#include "setup.h"

#ifndef CONSOLE_UART
#	define CONSOLE_UART uart0
#endif

#ifndef CONSOLE_MAX_CMD_LENGTH
#	define CONSOLE_MAX_CMD_LENGTH (UART_RX_BUFFER_SIZE - 1)
#endif

namespace console
{

DECLARE_PGM_STR (str_console_prompt, "> ");
DECLARE_PGM_STR (str_console_endl, "\r\n");
DECLARE_PGM_STR (str_console_bs, "\x08 \x08");

static uint8_t _command_len = 0;
static callback_t _handler = NULL;
static char _command [CONSOLE_MAX_CMD_LENGTH];

void endl ()
{
	CONSOLE_UART::send_string_p (str_console_endl);
}

inline void show_prompt ()
{
	endl ();
	CONSOLE_UART::send_string_p (str_console_prompt);
	_command_len = 0;
}

void set_callback (callback_t handler)
{
	_handler = handler;
}

char *str_argument (uint8_t position, const char *def)
{
	uint8_t offs = 0;
	while (_command [offs] && _command [offs] == ' ') offs ++;
	for (uint8_t i = 0; i < position; i ++)
	{
		while (_command [offs] && _command [offs] != ' ') offs ++;
		while (_command [offs] && _command [offs] == ' ') offs ++;
	}
	return &_command [offs];
}

int32_t int_argument (uint8_t position)
{
	return atol (str_argument (position));
}

bool _process_byte ()
{
	uint16_t data = CONSOLE_UART::receive ();
	if (data & 0xff00) return false;
	uint8_t byte = data & 0xff;
	endl ();
	switch (byte)
	{
		case KEY_ENTER:
			PORTB = 0x00;
			if (_command_len)
			{
				_command [_command_len] = 0;
				endl ();
				if (_handler) _handler (_command);
			}
			show_prompt ();
			return true;
		case KEY_DEL:
		case KEY_BS:
			if (!_command_len) return true;
			_command_len --;
			CONSOLE_UART::send_string_p (str_console_bs);
			return true;
		default:
			if (byte < 0x20 || _command_len == CONSOLE_MAX_CMD_LENGTH - 2)
			{
				CONSOLE_UART::send (0x07);
				return true;
			}
			_command [_command_len ++] = byte;
			CONSOLE_UART::send (byte);
	}
	return true;
}

void tick ()
{
	while (_process_byte ())
		;
}

}
