/************************************************************************
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
*************************************************************************/

#ifndef RTOS_CORE_H_
#define RTOS_CORE_H_

#include <stdint.h>
#include "base.h"

#define RTOS_TIMER0 0
#define RTOS_TIMER1 1
#define RTOS_TIMER2 2

#define RCP_1 	 0b001
#define RCP_8 	 0b010
#define RCP_32 	 0b011
#define RCP_64 	 0b100
#define RCP_128  0b101
#define RCP_256  0b110
#define RCP_1024 0b111

#include "setup.h"

#ifndef RTOS_CORE_TIMER
#	define RTOS_CORE_TIMER 0
#endif

#ifndef RTOS_CORE_PRESCALER
#	define RTOS_CORE_PRESCALER RCP_64
#endif

#if RTOS_CORE_PRESCALER == RCP_1
#	define RTOS_CORE_PRESCALER_VALUE 1
#elif RTOS_CORE_PRESCALER == RCP_8
#	define RTOS_CORE_PRESCALER_VALUE 8
#elif RTOS_CORE_PRESCALER == RCP_32
#	define RTOS_CORE_PRESCALER_VALUE 32
#elif RTOS_CORE_PRESCALER == RCP_64
#	define RTOS_CORE_PRESCALER_VALUE 64
#elif RTOS_CORE_PRESCALER == RCP_128
#	define RTOS_CORE_PRESCALER_VALUE 128
#elif RTOS_CORE_PRESCALER == RCP_256
#	define RTOS_CORE_PRESCALER_VALUE 256
#elif RTOS_CORE_PRESCALER == RCP_1024
#	define RTOS_CORE_PRESCALER_VALUE 1024
#endif

#ifndef RTOS_CORE_OCR
#	define RTOS_CORE_OCR (F_CPU / RTOS_CORE_PRESCALER_VALUE / 1000)
#endif


#include "platform.h"
#include "platform/shared.h"

namespace rtos
{

class task_t;

typedef void (* callback_t) (task_t *task);

class task_t: public _class
{
	public:
		callback_t callback;
		uint32_t interval, idle;
		uint16_t max_runs, runs;
		bool single, sleeping, running;
		task_t *prev, *next;

		task_t (callback_t _callback, uint32_t _interval, uint16_t _max_runs, bool _single, bool _sleeping, task_t *_prev, task_t *_next)
			: callback (_callback), interval (_interval), idle (0), max_runs (_max_runs), runs (0),
			  single (_single), sleeping (_sleeping), running (false),
			  prev (_prev), next (_next)
		{}

		inline void sleep ()
		{
			sleeping = true;
		}

		inline void wakeup ()
		{
			sleeping = false;
		}

		void run ();
};

volatile static uint32_t ticks = 0;
void start ();
task_t *add (callback_t callback, uint32_t interval, uint8_t max_runs = 0, bool single = true, bool sleeping = false);
void remove (task_t *task);

inline void tick ();


}


#endif /* RTOS_CORE_H_ */
