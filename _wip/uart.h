#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "bsp/board_config.h"

typedef enum {
    UART_MODE_ASYNC,
    UART_MODE_SYNC,
    UART_MODE_MSPIM
} uart_mode_t;

typedef enum{
  UART_FRAME_SIZE_5 = 0b000,
  UART_FRAME_SIZE_6 = 0b001,
  UART_FRAME_SIZE_7 = 0b010,
  UART_FRAME_SIZE_8 = 0b011,  // UCSZ01=1, UCSZ00=1
  UART_FRAME_SIZE_9 = 0b111
}framesize_t;

typedef enum{
  UART_PARITY_DISABLED = 0b00,
  UART_PARITY_EVEN     = 0b10,
  UART_PARITY_ODD      = 0b11
}parity_t;


typedef enum{
  SPI_MODE_0 = 0b00,  // UCPHA=0, UCPOL=0
  SPI_MODE_1 = 0b01,  // UCPHA=1, UCPOL=0
  SPI_MODE_2 = 0b10,  // UCPHA=0, UCPOL=1
  SPI_MODE_3 = 0b11   // UCPHA=1, UCPOL=1
}spi_mode_t;


void uart_init(uart_config_t *config);
void uart_transmit(uint16_t data); 
uint8_t uart_receive(void);

uint8_t uart_data_available(void);
void    uart_print(const char *str);

#endif