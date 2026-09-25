#ifndef UART_H
#define UART_H

#include <stdint.h>
#include "status.h"

#ifndef UART_FRAMESIZE
  #error "UART_FRAMESIZE must be defined by Build-System"
#endif

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

typedef struct {
    uint32_t    baudrate;
    uart_mode_t mode;
    framesize_t framesize;
    parity_t    parity;
    uint8_t     two_stop_bits;
    uint8_t     async_double_speed;
    spi_mode_t  spi_mode;

} uart_config_t;

// Raw link-layer error bits observed on a received byte (from UCSR0A).
// More than one can be set at once, so they're independent flags, not an
// exclusive status_t code -- the caller decides which one(s) matter to them.
#define UART_ERR_OVERRUN (1 << 0)
#define UART_ERR_FRAME   (1 << 1)
#define UART_ERR_PARITY  (1 << 2)

typedef struct {
    status_t status;      // transport result: STATUS_OK or STATUS_ERR_TIMEOUT
    uint8_t  link_errors;  // bitmask of UART_ERR_* flags for the byte received (0 if none)
} uart_result_t;

void     uart_init(uart_config_t *config);
status_t uart_reinit(uart_config_t *config);
#if UART_FRAMESIZE == 9
  status_t      uart_transmit(uint16_t data);
  uart_result_t uart_receive(uint16_t *data);
#else
  status_t      uart_transmit(uint8_t data);
  uart_result_t uart_receive(uint8_t *data);
#endif

void     uart_flush(void);
uint8_t  uart_data_available(void);
status_t uart_print(const char *str);

#endif