#include "uart.h"
#include "uart_internal.h"
#include "board_config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

// Generous safety-net bound for the UDRE0/RXC0/TXC0 busy-waits below. Unlike
// ADC_poll's timeout (a fixed conversion clock count from the datasheet), the
// real worst case here depends on the runtime-configured baud rate, so this
// isn't computed -- just large enough that a correctly wired UART never hits it.
#define UART_TIMEOUT_LOOPS 50000u

/*
uart_operating_mode = 0 : Asynchronous normal mode (U2Xn = 0)
uart_operating_mode = 1 : Asynchronous double speed mode (U2Xn = 1)
uart_operating_mode = 2 : Synchronous master mode

*/


void set_data_frame_size(framesize_t * framesize){

  UCSR0B = (UCSR0B & ~(1<<UCSZ02)) | ((((*framesize) >> 2) & 1) << UCSZ02);
  UCSR0C = (UCSR0C & ~((1<<UCSZ01)|(1<<UCSZ00))) | (((*framesize) & 0x03) << UCSZ00);

}

#if UART_FRAMESIZE == 9
  status_t      uart_transmit(uint16_t data)     { return uart_transmit9(data);}
  uart_result_t uart_receive(uint16_t *data)     { return uart_receive9(data);}
#else
  status_t      uart_transmit(uint8_t data)      { return uart_transmit8(data);}
  uart_result_t uart_receive(uint8_t *data)      { return uart_receive8(data);}
#endif

void
set_baud(uart_config_t * config){
  uint32_t baud = config->baudrate ? config->baudrate : BAUDRATE;

  uint16_t ubrrn;

  if(config->mode == UART_MODE_ASYNC && !config->async_double_speed){
    ubrrn  = (F_CPU/ (16*baud)) - 1;
  }else if(config->mode == UART_MODE_ASYNC && config->async_double_speed){
    ubrrn  = (F_CPU/ (8*baud)) - 1;
  }else{
    ubrrn  = (F_CPU/ (2*baud)) - 1;
  }

  UBRR0H = ubrrn >> 8;
  UBRR0L = ubrrn & 0xFF;
}

void 
set_stop_bits(uint8_t two_stop_bits){
  // set 2 or 1 stop bit(s)
  UCSR0C &= ~(1 << USBS0);
  UCSR0C |= ((two_stop_bits & 1)<<USBS0);
}

void
set_parity_mode(parity_t * parity){
  // clear previous and set new parity mode
  UCSR0C &= ~((1<<UPM01) | (1<<UPM00));
  UCSR0C |= (((((*parity) >> 1) & 1) << UPM01) | (((*parity)  & 1) << UPM00));
}

void
uart_init(uart_config_t *config){
  // DDR_XCKn preferably set before UBRR
  uint8_t sreg_backup = SREG;

  cli();
  
  switch(config->mode){
    case UART_MODE_ASYNC:

      UCSR0A &= ~(1 << U2X0);

      UCSR0C &= ~((1<<UMSEL00) | (1<< UMSEL01) | (1<<UCPOL0));

      if(config->async_double_speed) UCSR0A |= (1<<U2X0);
      break;
    case UART_MODE_SYNC:
      
      UCSR0A &= ~(1<<U2X0);

      UCSR0C &= ~(1<<UMSEL01);
      UCSR0C |= (1<<UMSEL00);

      break;
    case UART_MODE_MSPIM:
      // WIP
      UBRR0 = 0;

      DDRD |= (1<<XCK);//set XCK output mode
 
      UCSR0A &= ~(1<<U2X0);

      UCSR0C |= (1<<UMSEL01 ) | (1<<UMSEL00);
      
      break;
    default:

      UCSR0C &= ~(1<<UCPOL0);

      UCSR0C &= ~((1<<UMSEL00) | (1<< UMSEL01));

      break;
  }

  // TODO MASTER SPI+
  // TODO UART STATISTICS ( ERROR AND Übertragung)

  set_parity_mode(&config->parity);
  set_stop_bits(config->two_stop_bits);
  set_data_frame_size(&config->framesize);
  set_baud(config);

  //Enable Tansmit and Recieve
  UCSR0B |= (1 << RXEN0) | (1 << TXEN0);

  SREG = sreg_backup;
}

status_t
uart_reinit(uart_config_t *config){
  /* Falls init noch nie aufgerufen TXC0 noch nie gesetzt und while könnte
  sich aufhängen -- caller must have called uart_init first */
  if (!(UCSR0B & ((1 << RXEN0) | (1 << TXEN0)))) return STATUS_ERR_PARAM;
  // Vor Re-Init sicherstellen: kein laufender Transfer, kein ungelesenes RX-Byte
  uart_flush();

  uint16_t timeout = UART_TIMEOUT_LOOPS;
  while (!(UCSR0A & (1 << TXC0)) && --timeout){/*wait for pending transmit to complete*/}
  if (timeout == 0) return STATUS_ERR_TIMEOUT;

  uart_init(config);
  return STATUS_OK;
}

// void
// uart_transmit(uint16_t data){
//   /* Wait for empty transmit buffer */
//   while (!(UCSR0A & (1<<UDRE0)));
  
  
//   if(UCSR0B & 1<<UCSZ02){
//     /* Copy 9th bit to TXB8 */
//     UCSR0B &= ~(1<<TXB80);
//     if (data & 0x0100){
//       UCSR0B |= (1<<TXB80);
//     }
//   }
//   /* Put data into buffer, sends the data */
//   UCSR0A |= (1<<TXC0);
//   UDR0 = (uint8_t) data;
// }

// Maps the raw UCSR0A error flags for a just-received byte to the
// independent UART_ERR_* bitmask (see uart.h) -- no priority imposed here,
// callers decide which bit(s) they care about.
static uint8_t uart_link_errors(uint8_t status_reg){
    uint8_t errors = 0;
    if (status_reg & (1 << DOR0)) errors |= UART_ERR_OVERRUN;
    if (status_reg & (1 << FE0))  errors |= UART_ERR_FRAME;
    if (status_reg & (1 << UPE0)) errors |= UART_ERR_PARITY;
    return errors;
}

/* Überträgt Daten im 5- bis 8-Bit-UART-Modus */
status_t uart_transmit8(uint8_t data)
{
    /* Warten bis Sendepuffer frei ist */
    uint16_t timeout = UART_TIMEOUT_LOOPS;
    while (!(UCSR0A & (1 << UDRE0)) && --timeout){}
    if (timeout == 0) return STATUS_ERR_TIMEOUT;

    /* TX Complete Flag löschen */
    UCSR0A |= (1 << TXC0);

    /* Daten senden */
    UDR0 = data;
    return STATUS_OK;
}


/* Überträgt Daten im 9-Bit-UART-Modus */
status_t uart_transmit9(uint16_t data)
{
    /* Warten bis Sendepuffer frei ist */
    uint16_t timeout = UART_TIMEOUT_LOOPS;
    while (!(UCSR0A & (1 << UDRE0)) && --timeout){}
    if (timeout == 0) return STATUS_ERR_TIMEOUT;

    /* 9. Bit setzen oder löschen */
    if (data & 0x0100)
        UCSR0B |= (1 << TXB80);
    else
        UCSR0B &= ~(1 << TXB80);

    /* TX Complete Flag löschen */
    UCSR0A |= (1 << TXC0);

    /* Untere 8 Bit senden */
    UDR0 = (uint8_t)data;
    return STATUS_OK;
}

uart_result_t
uart_receive8(uint8_t *data){
  uart_result_t result = { STATUS_OK, 0 };

  /* Wait for data to be received */
  uint16_t timeout = UART_TIMEOUT_LOOPS;
  while (!(UCSR0A & (1<<RXC0)) && --timeout){}
  if (timeout == 0){
    result.status = STATUS_ERR_TIMEOUT;
    return result;
  }

  uint8_t status_reg = UCSR0A;
  *data = UDR0;

  result.link_errors = uart_link_errors(status_reg);
  return result;
}

uart_result_t
uart_receive9(uint16_t *data){
  uart_result_t result = { STATUS_OK, 0 };

  uint16_t timeout = UART_TIMEOUT_LOOPS;
  while (!(UCSR0A & (1<<RXC0)) && --timeout){}
  if (timeout == 0){
    result.status = STATUS_ERR_TIMEOUT;
    return result;
  }

  uint8_t status_reg = UCSR0A;
  uint8_t resh   = UCSR0B;
  uint8_t resl   = UDR0;

  /* Filter the 9th bit, then return */
  *data = (uint16_t) (((resh >> RXB80 )&1) << 8) | resl;

  result.link_errors = uart_link_errors(status_reg);
  return result;
}

void uart_flush(void) {
    uint8_t dummy;
    while (UCSR0A & (1 << RXC0)) {
        dummy = UDR0;  // auslesen und wegwerfen
    }
    (void)dummy;
}

uint8_t
uart_data_available(void){
  return (UCSR0A & (1 << RXC0)) != 0;
}


status_t uart_print(const char * str){
  status_t result = STATUS_OK;
  while(*str){
    result = uart_transmit((uint8_t)*str++);
    if (result != STATUS_OK) return result;
  }
  return result;
}