#include "core.h"

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "hardware/uart/uart.h"

namespace rtos
{


static task_t *_last = NULL;
static task_t *_first = NULL;
volatile static bool mutex = false;
volatile static uint32_t _dropped = 0;

#define acquire_mutex() { while (mutex) {} mutex = true; }
#define release_mutex() { mutex = false; }

void task_t::run ()
{
	if (sleeping || (single && running) || (idle < interval)) return;
	idle = idle - interval;
	running = true;
	if (callback) callback (this);
	running = false;
	if (++ runs >= max_runs && max_runs)
		remove (this);
}

void start ()
{
	RTOS_TIMER_INIT ();
	sei ();
}

task_t *add (callback_t callback, uint32_t interval, uint8_t max_runs, bool single, bool sleeping)
{
	task_t *result = new task_t (callback, interval, max_runs, single, sleeping, _last, NULL);
	if (!result) return NULL;

	acquire_mutex ();
	if (_last) _last->next = result;
	if (!_first) _first = result;
	_last = result;
	release_mutex ();

	return result;
}

void remove (task_t *task)
{
	if (!task) return;

	acquire_mutex ();
	if (task->next) task->next->prev = task->prev;
	if (task->prev) task->prev->next = task->next;
	if (task == _first) _first = task->next;
	else if (task == _last) _last = task->prev;
	release_mutex ();

	delete task;
}

void tick ()
{
	ticks ++;
	_dropped ++;

	if (mutex) return;
	mutex = true;

	task_t *task = _first;
	while (task)
	{
		task->idle += _dropped;
		release_mutex ();
		task->run ();
		acquire_mutex ();
		task = task->next;
	}
	_dropped = 0;
	release_mutex ();
}

#ifdef RTOS_CORE_ISR
ISR (RTOS_TIMER_VECTOR)
{
	tick ();
}
#endif

}