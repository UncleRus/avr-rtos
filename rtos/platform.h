#ifndef RTOS_PLATFORM_H_
#define RTOS_PLATFORM_H_

#if defined (__AVR_ATmega16__)
#	include "platform/atmega16.h"
#elif defined (__AVR_ATmega8__)
#	include "platform/atmega8.h"
#else
#	error Unknwon processor
#endif

#endif /* RTOS_PLATFORM_H_ */
