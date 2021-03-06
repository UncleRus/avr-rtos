#include "core.h"

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "hardware/uart/uart.h"

namespace rtos
{


static task_t *last = NULL;
static task_t *first = NULL;
volatile static bool mutex = false;
volatile static uint32_t skipped = 0;

#define acquire_mutex() { while (mutex) {} mutex = true; }
#define release_mutex() { mutex = false; }

task_t *task_t::run ()
{
	if (sleeping || (single && running) || (idle < interval)) return next;
	idle -= interval;
	running = true;
	if (callback) callback (this);
	running = false;
	runs ++;
	if (max_runs && runs >= max_runs)
	{
		task_t *result = next;
		remove (this);
		return result;
	}
	return next;
}

void start ()
{
	RTOS_TIMER_INIT ();
	sei ();
}

task_t *add (callback_t callback, uint32_t interval, uint8_t max_runs, bool single, bool sleeping)
{
	task_t *result = new task_t (callback, interval, max_runs, single, sleeping, last, NULL);
	if (!result) return NULL;

	acquire_mutex ();
	if (last) last->next = result;
	if (!first) first = result;
	last = result;
	release_mutex ();

	return result;
}

void remove (task_t *task)
{
	if (!task) return;

	acquire_mutex ();
	if (task->next) task->next->prev = task->prev;
	if (task->prev) task->prev->next = task->next;
	if (task == first) first = task->next;
	if (task == last) last = task->prev;
	release_mutex ();

	delete task;
}

inline void tick ()
{
	ticks ++;
	skipped ++;

	if (mutex) return;
	mutex = true;

	task_t *task = first;
	while (task)
	{
		task->idle += skipped;
		release_mutex ();
		task = task->run ();
		acquire_mutex ();
	}
	skipped = 0;
	release_mutex ();
}

#ifdef RTOS_CORE_ISR
ISR (RTOS_TIMER_VECTOR)
{
	tick ();
}
#endif

}
