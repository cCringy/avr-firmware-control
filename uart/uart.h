#ifndef UART_H
#define UART_H

#include <stqdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

typedef enum {
    UART_MODE_ASYNC_NORMAL = 1,
    UART_MODE_ASYNC_DOUBLE = 2,
    UART_MODE_SYNC_MASTER  = 3
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

typedef struct {
    uint32_t    baudrate;
    uart_mode_t mode;
    framesize_t framesize;
    parity_t    parity;
    uint8_t     two_stop_bits;
} uart_config_t;


void    uart_init(uint32_t baudrate, uart_mode_t mode);
void    uart_transmit(uint8_t data);
uint8_t uart_receive(void);
uint8_t uart_data_available(void);
void    uart_print(const char *str);

#endif