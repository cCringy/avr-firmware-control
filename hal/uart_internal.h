#ifndef UART_INTERNAL_H
#define UART_INTERNAL_H

#include <stdint.h>
#include "uart.h"

status_t      uart_transmit8(uint8_t data);
status_t      uart_transmit9(uint16_t data);
uart_result_t uart_receive8(uint8_t *data);
uart_result_t uart_receive9(uint16_t *data);

#endif