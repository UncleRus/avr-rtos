#include "sdac.h"
#include <util/delay.h>

#include "setup.h"

#ifndef SDAC_SCLK_DDR
#	define SDAC_SCLK_DDR DDRD
#endif
#ifndef SDAC_SCLK_PORT
#	define SDAC_SCLK_PORT PORTD
#endif
#ifndef SDAC_SCLK_BIT
#	define SDAC_SCLK_BIT PD5
#endif

#ifndef SDAC_SYNC_DDR
#	define SDAC_SYNC_DDR DDRD
#endif
#ifndef SDAC_SYNC_PORT
#	define SDAC_SYNC_PORT PORTD
#endif
#ifndef SDAC_SYNC_BIT
#	define SDAC_SYNC_BIT PD6
#endif

#ifndef SDAC_DATA_DDR
#	define SDAC_DATA_DDR DDRD
#endif
#ifndef SDAC_DATA_PORT
#	define SDAC_DATA_PORT PORTD
#endif
#ifndef SDAC_DATA_BIT
#	define SDAC_DATA_BIT PD7
#endif

#ifndef SDAC_DELAY
#	define SDAC_DELAY 10
#endif


#include "rtos/core.h"

namespace sdac
{

volatile static bool mutex = false;

#define acquire_mutex() { while (mutex) {} mutex = true; }
#define release_mutex() { mutex = false; }


#define sync() { SDAC_SYNC_PORT |= _BV (SDAC_SYNC_BIT); _delay_us (SDAC_DELAY); SDAC_SYNC_PORT &= ~_BV (SDAC_SYNC_BIT); }
#define send_bit(b) { \
	SDAC_DATA_PORT &= ~_BV (SDAC_DATA_BIT); \
	SDAC_DATA_PORT |= (b & 0xfffe) << SDAC_DATA_BIT; \
	SDAC_SCLK_PORT |= _BV (SDAC_SCLK_BIT); \
	_delay_us (SDAC_DELAY); \
	SDAC_SCLK_PORT &= ~_BV (SDAC_SCLK_BIT); \
	_delay_us (SDAC_DELAY); \
}

void init ()
{
	SDAC_SCLK_DDR |= _BV (SDAC_SCLK_BIT);
	SDAC_SYNC_DDR |= _BV (SDAC_SYNC_BIT);
	SDAC_DATA_DDR |= _BV (SDAC_DATA_BIT);

	SDAC_SCLK_PORT &= ~_BV (SDAC_SCLK_BIT);
	SDAC_SYNC_PORT |= _BV (SDAC_SYNC_BIT);
	SDAC_DATA_PORT &= ~_BV (SDAC_DATA_BIT);
}

void set (uint16_t value)
{
	acquire_mutex ();


	sync ();
	for (uint8_t i = 0; i < 16; i ++)
		send_bit (value >> i);
	_delay_us (SDAC_DELAY);

	release_mutex ();
}


}
