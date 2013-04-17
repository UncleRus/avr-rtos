#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>

namespace uart
{

#define UART_RX_BUFFER_MASK (UART_RX_BUFFER_SIZE - 1)
#define UART_TX_BUFFER_MASK (UART_TX_BUFFER_SIZE - 1)

#if (UART_RX_BUFFER_SIZE & UART_RX_BUFFER_MASK)
#error RX buffer size is not a power of 2
#endif
#if (UART_TX_BUFFER_SIZE & UART_TX_BUFFER_MASK)
#error TX buffer size is not a power of 2
#endif

/*
 *  module global variables
 */
static volatile uint8_t _tx_buffer [UART_TX_BUFFER_SIZE];
static volatile uint8_t _rx_buffer [UART_RX_BUFFER_SIZE];
static volatile uint8_t _tx_head;
static volatile uint8_t _tx_tail;
static volatile uint8_t _rx_head;
static volatile uint8_t _rx_tail;
static volatile uint8_t last_error;

#if defined(ATMEGA_USART1)
static volatile uint8_t _tx_buffer1 [UART_TX_BUFFER_SIZE];
static volatile uint8_t _rx_buffer1 [UART_RX_BUFFER_SIZE];
static volatile uint8_t _tx_head1;
static volatile uint8_t _tx_tail1;
static volatile uint8_t _rx_head1;
static volatile uint8_t _rx_tail1;
static volatile uint8_t last_error1;
#endif

ISR (UART0_RECEIVE_INTERRUPT)
{
	uint8_t _head;
	uint8_t data;
	uint8_t usr;
	uint8_t lastRxError;

	/* read UART status register and UART data register */
	usr = UART0_STATUS;
	data = UART0_DATA;

	/* */
#if defined(AT90_UART)
	lastRxError = (usr & (_BV (FE)|_BV (DOR)));
#elif defined(ATMEGA_USART)
	lastRxError = (usr & (_BV (FE) | _BV (DOR)));
#elif defined(ATMEGA_USART0)
	lastRxError = (usr & (_BV (FE0)|_BV (DOR0)));
#elif defined (ATMEGA_UART)
	lastRxError = (usr & (_BV (FE)|_BV (DOR)));
#endif

	/* calculate buffer index */
	_head = (_rx_head + 1) & UART_RX_BUFFER_MASK;

	if (_head == _rx_tail)
		/* error: receive buffer overflow */
		lastRxError = UART_BUFFER_OVERFLOW >> 8;
	else
	{
		/* store new index */
		_rx_head = _head;
		/* store received data in buffer */
		_rx_buffer [_head] = data;
	}
	last_error |= lastRxError;
}

ISR (UART0_TRANSMIT_INTERRUPT)
{
	uint8_t _tail;

	if (_tx_head == _tx_tail)
	{
		/* tx buffer empty, disable UDRE interrupt */
		UART0_CONTROL &= ~_BV (UART0_UDRIE);
		return;
	}
	/* calculate and store new buffer index */
	_tail = (_tx_tail + 1) & UART_TX_BUFFER_MASK;
	_tx_tail = _tail;
	/* get one byte from buffer and write it to UART */
	UART0_DATA = _tx_buffer [_tail]; /* start transmission */
}

void init (uint16_t baud_rate)
{
	_tx_head = 0;
	_tx_tail = 0;
	_rx_head = 0;
	_rx_tail = 0;

#if defined(AT90_UART)
	/* set baud rate */
	UBRR = (uint8_t)baud_rate;

	/* enable UART receiver and transmmitter and receive complete interrupt */
	UART0_CONTROL = _BV (RXCIE)|_BV (RXEN)|_BV (TXEN);

#elif defined (ATMEGA_USART)
	/* Set baud rate */
	if (baud_rate & 0x8000)
	{
		UART0_STATUS = (1 << U2X);  //Enable 2x speed
		baud_rate &= ~0x8000;
	}
	UBRRH = (uint8_t) (baud_rate >> 8);
	UBRRL = (uint8_t) baud_rate;

	/* Enable USART receiver and transmitter and receive complete interrupt */
	UART0_CONTROL =	_BV (RXCIE) | (1 << RXEN) | (1 << TXEN);

	/* Set frame format: asynchronous, 8data, no parity, 1stop bit */
#ifdef URSEL
	UCSRC = (1 << URSEL) | (3 << UCSZ0);
#else
	UCSRC = (3<<UCSZ0);
#endif

#elif defined (ATMEGA_USART0)
	/* Set baud rate */
	if (baud_rate & 0x8000)
	{
		UART0_STATUS = (1<<U2X0);  //Enable 2x speed
		baud_rate &= ~0x8000;
	}
	UBRR0H = (uint8_t)(baud_rate>>8);
	UBRR0L = (uint8_t) baud_rate;

	/* Enable USART receiver and transmitter and receive complete interrupt */
	UART0_CONTROL = _BV (RXCIE0)|(1<<RXEN0)|(1<<TXEN0);

	/* Set frame format: asynchronous, 8data, no parity, 1stop bit */
#ifdef URSEL0
	UCSR0C = (1<<URSEL0)|(3<<UCSZ00);
#else
	UCSR0C = (3<<UCSZ00);
#endif

#elif defined (ATMEGA_UART)
	/* set baud rate */
	if (baud_rate & 0x8000)
	{
		UART0_STATUS = (1<<U2X);  //Enable 2x speed
		baud_rate &= ~0x8000;
	}
	UBRRHI = (uint8_t)(baud_rate>>8);
	UBRR = (uint8_t) baud_rate;

	/* Enable UART receiver and transmitter and receive complete interrupt */
	UART0_CONTROL = _BV (RXCIE)|(1<<RXEN)|(1<<TXEN);
#endif
}

uint16_t getc ()
{
	uint8_t _tail;
	uint8_t data;

	if (_rx_head == _rx_tail)
		return UART_NO_DATA; /* no data available */

	/* calculate /store buffer index */
	_tail = (_rx_tail + 1) & UART_RX_BUFFER_MASK;
	_rx_tail = _tail;

	/* get data from receive buffer */
	data = _rx_buffer [_tail];

	data = (last_error << 8) + data;
	last_error = 0;
	return data;
}

void putc (uint8_t data)
{
	uint8_t _head;

	_head = (_tx_head + 1) & UART_TX_BUFFER_MASK;

	while (_head == _tx_tail)
		;/* wait for free space in buffer */

	_tx_buffer [_head] = data;
	_tx_head = _head;

	/* enable UDRE interrupt */
	UART0_CONTROL |= _BV (UART0_UDRIE);

}

void puts (const char *s)
{
	while (*s)
		putc (*s ++);
}

void puts_p (const char *progmem_s)
{
	register char c;

	while ((c = pgm_read_byte (progmem_s ++)))
		putc (c);
}

void puti (int data, int radix)
{
	char buffer [17];
	puts (itoa (data, buffer, radix));
}

#if defined (ATMEGA_USART1)

ISR (UART1_RECEIVE_INTERRUPT)
{
	uint8_t _head;
	uint8_t data;
	uint8_t usr;
	uint8_t lastRxError;

	/* read UART status register and UART data register */
	usr = UART1_STATUS;
	data = UART1_DATA;

	/* */
	lastRxError = (usr & (_BV (FE1) | _BV (DOR1)));

	/* calculate buffer index */
	_head = (_rx_head1 + 1) & UART_RX_BUFFER_MASK;

	if (_head == _rx_tail1)
	{
		/* error: receive buffer overflow */
		lastRxError = UART_BUFFER_OVERFLOW >> 8;
	}
	else
	{
		/* store new index */
		_rx_head1 = _head;
		/* store received data in buffer */
		_rx_buffer1 [_head] = data;
	}
	last_error1 |= lastRxError;
}

ISR (UART1_TRANSMIT_INTERRUPT)
{
	uint8_t _tail;

	if (_tx_head1 == _tx_tail1)
	{
		/* tx buffer empty, disable UDRE interrupt */
		UART1_CONTROL &= ~_BV (UART1_UDRIE);
		return;
	}
	/* calculate and store new buffer index */
	_tail = (_tx_tail1 + 1) & UART_TX_BUFFER_MASK;
	_tx_tail1 = _tail;
	/* get one byte from buffer and write it to UART */
	UART1_DATA = _tx_buffer1 [_tail]; /* start transmission */
}

void init1 (uint16_t baud_rate)
{
	_tx_head1 = 0;
	_tx_tail1 = 0;
	_rx_head1 = 0;
	_rx_tail1 = 0;

	/* Set baud rate */
	if (baud_rate & 0x8000)
	{
		UART1_STATUS = (1 << U2X1);  //Enable 2x speed
		baud_rate &= ~0x8000;
	}
	UBRR1H = (uint8_t) (baud_rate >> 8);
	UBRR1L = (uint8_t) baud_rate;

	/* Enable USART receiver and transmitter and receive complete interrupt */
	UART1_CONTROL = _BV (RXCIE1) | (1 << RXEN1) | (1 << TXEN1);

	/* Set frame format: asynchronous, 8data, no parity, 1stop bit */
#ifdef URSEL1
	UCSR1C = (1<<URSEL1)|(3<<UCSZ10);
#else
	UCSR1C = (3 << UCSZ10);
#endif
}

uint16_t getc1 ()
{
	uint8_t _tail;
	uint8_t data;

	if (_rx_head1 == _rx_tail1)
		return UART_NO_DATA; /* no data available */

	/* calculate /store buffer index */
	_tail = (_rx_tail1 + 1) & UART_RX_BUFFER_MASK;
	_rx_tail1 = _tail;

	/* get data from receive buffer */
	data = _rx_buffer1 [_tail];

	data = (last_error1 << 8) + data;
	last_error = 0;
	return data;

}

void putc1 (uint8_t data)
{
	uint8_t _head;

	_head = (_tx_head1 + 1) & UART_TX_BUFFER_MASK;

	while (_head == _tx_tail1)
		;/* wait for free space in buffer */

	_tx_buffer1 [_head] = data;
	_tx_head1 = _head;

	/* enable UDRE interrupt */
	UART1_CONTROL |= _BV (UART1_UDRIE);
}

void puts1 (const char *s)
{
	while (*s)
	putc1 (*s ++);
}

void puts_p1 (const char *progmem_s)
{
	register char c;

	while ((c = pgm_read_byte (progmem_s++)))
		putc1 (c);
}

void puti (int data, int radix)
{
	char buffer [17];
	puts (itoa (data, buffer, radix));
}
#endif

}
