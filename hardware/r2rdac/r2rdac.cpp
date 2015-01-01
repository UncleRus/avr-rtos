#include "r2rdac.h"
#include <avr/io.h>

namespace r2rdac
{


void init ()
{
	DDRD = 0xff;
	DDRC |= _BV (PC2) | _BV (PC3);
	set (0);
}

void set (uint16_t value)
{
	uint8_t old = PORTC;
	PORTD = value & 0xff;
	PORTC = ((value & 0x300) >> 6) | (old & 0xf3);
}


}
