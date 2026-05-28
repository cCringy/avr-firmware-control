/*! \file uart.h
 * \brief Minimal UART library used for debugging and demo.
 */
#ifndef UART_H_
#define UART_H_

#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>

/*! \brief initialize UART peripheral and interrupts. */	
void uart_init();

/*! \brief Transmit single character. */	
void uart_transmit(unsigned char data);

/*! \brief return received data from buffer. */	
unsigned char uart_receive(void);

/*! \brief Dummy (does nothing) */	
void uart_flush(void);

/*! \brief Send one character to the UART. */
int	uart_putchar(char c, FILE *stream);

/*! \brief Forward output stream to UART. */	
int uart_printf(char var, FILE *stream);

/*! \brief Size of internal line buffer used by uart_getchar(). */
#define RX_BUFSIZE 80

/*! \brief Receive one character from the UART.
 * The actual reception is line-buffered,
 * and one character is returned from the buffer
 * at each invocation.
 */
int	uart_getchar(FILE *stream);

#endif /* UART_H_ */