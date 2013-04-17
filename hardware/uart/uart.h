#ifndef UART_H_
#define UART_H_

#include "setup.h"
#include <avr/io.h>

#ifndef UART_RX_BUFFER_SIZE
#	define UART_RX_BUFFER_SIZE 64
#endif

#ifndef UART_TX_BUFFER_SIZE
#	define UART_TX_BUFFER_SIZE 64
#endif

#define UART_BAUD_SELECT(baud_rate)  (((F_CPU) + 8UL * (baud_rate)) / (16UL * (baud_rate)) -1UL)
#define UART_BAUD_SELECT_DOUBLE_SPEED(baud_rate) ( ((((F_CPU) + 4UL * (baud_rate)) / (8UL * (baud_rate)) -1UL)) | 0x8000)

#if ((UART_RX_BUFFER_SIZE + UART_TX_BUFFER_SIZE) >= (RAMEND - 0x60))
#	error "size of UART_RX_BUFFER_SIZE + UART_TX_BUFFER_SIZE larger than size of SRAM"
#endif

#define UART_FRAME_ERROR      0x1000              /* Framing Error by UART       */
#define UART_OVERRUN_ERROR    0x0800              /* Overrun condition by UART   */
#define UART_PARITY_ERROR     0x0400              /* Parity Error by UART        */
#define UART_BUFFER_OVERFLOW  0x0200              /* receive ringbuffer overflow */
#define UART_NO_DATA          0x0100              /* no receive data available   */

#include "platform.h"

namespace uart
{

void init (uint16_t baud_rate);
uint16_t getc ();
void putc (uint8_t data);
void puti (int data, int radix = 10);
void puts (const char *s);
void puts_p (const char *progmem_s);
#define puts_P(__s) puts_p (PSTR (__s))

#ifdef ATMEGA_USART1
void init1 (unsigned int baud_rate);
uint16_t getc1 ();
void putc1 (unsigned char data);
void puti1 (int data, int radix)
void puts1 (const char *s);
void puts_p1 (const char *s);
#define puts_P1(__s) puts_p1 (PSTR (__s))
#endif

}
#endif /* UART_H_ */
