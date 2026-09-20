#ifndef UART_INTERNAL_H
#define UART_INTERNAL_H

#include <stdint.h>

void     uart_transmit8(uint8_t data);
void     uart_transmit9(uint16_t data);
uint8_t  uart_receive8(void);
uint16_t uart_receive9(void);

#endif