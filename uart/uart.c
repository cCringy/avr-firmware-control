#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

// Conservative Indusrty Standard
// slow enough for bad cables and fast enough for this usecase

#define BAUDRATE 9600UL  /*UL because if normal int on Arduino it will use 16 bit arithmetic
                         causing integer overflow*/
#define F_OSC 16000000UL

static uint8_t uart_last_error = 0;

/*
uart_operating_mode = 0 : Asynchronous normal mode (U2Xn = 0)
uart_operating_mode = 1 : Asynchronous double speed mode (U2Xn = 1)
uart_operating_mode = 2 : Synchronous master mode

*/


void set_data_frame_size(framesize_t * framesize){

  UCSR0B = (UCSR0B & ~(1<<UCSZ02)) | ((((*framesize) >> 2) & 1) << UCSZ02);
  UCSR0C = (UCSR0C & ~((1<<UCSZ01)|(1<<UCSZ00))) | (((*framesize) & 0x03) << UCSZ00);

}

void
set_baud(uint32_t baudrate,uart_mode_t operating_mode){
  uint32_t baud = baudrate ? baudrate : BAUDRATE;

  uint16_t ubrrn;

  if(!operating_mode){
    ubrrn  = (F_OSC/ (16*baud)) - 1;
  }else if( operating_mode == 1){
    ubrrn  = (F_OSC/ (8*baud)) - 1;
  }else{
    ubrrn  = (F_OSC/ (2*baud)) - 1;
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
  UCSR0C &= ~((1<<UPM1) | (1<<UPM0));
  UCSR0C |= (((((*parity) >> 1) & 1) << UPM1) | (((*parity)  & 1) << UPM0));
}

void
uart_init(uart_config_t *config){
  // DDR_XCKn preferably set before UBRR
  uint8_t sreg_backup = SREG;

  cli();
  
  switch( config->mode){

    case UART_MODE_ASYNC:

      UCSR0A &= ~(1 << U2X0);  

      UCSR0C &= ~((1<<UMSEL00) | (1<< UMSEL01) | (1<<UCPOL0));
      if(config->async_mode){UCSR0A |= (1<<U2X0);} 
      break;
    case UART_MODE_SYNC:
      
      break;
    case 
    case UART_MODE_SYNC_MASTER:
      
      UCSR0A &= ~(1<<U2X0);

      UCSR0C &= ~(1<<UMSEL01);

      break;
    default:

      UCSR0C &= ~(1<<UCPOL0);

      UCSR0C &= ~((1<<UMSEL00) | (1<< UMSEL01));

      break;
  }

  // TODO MASTER SPI+
  // TODO UART STATISTICS ( ERROR AND Übertragung)


  //Enable Tansmit and Recieve
  UCSR0B |= (1 << RXEN0) | (1 << TXEN0);

  set_parity_mode(&config->parity);
  set_stop_bits(config->two_stop_bits);
  set_data_frame_size(&config->framesize);
  set_baud(config->baudrate ? config->baudrate : BAUDRATE);

  SREG = sreg_backup;
}

void
uart_reinit(uart_config_t *config){
  /* Falls init noch nie aufgerufen TXC0  noch nie gesetzt und while könnte
  sich aufhängen*/
  if (!(UCSR0B & ((1 << RXEN0) | (1 << TXEN0)))) return;
  // Vor Re-Init sicherstellen: kein laufender Transfer, kein ungelesenes RX-Byte
  while ( !(UCSR0A & (1 << TXC0)) || (UCSR0A & (1 << RXC0)) );

  uart_init(config);
}

void
uart_transmit(uint16_t data){
  /* Wait for empty transmit buffer */
  while (!(UCSR0A & (1<<UDRE0)));
  
  
  if(UCSR0B & 1<<UCSZ02){
    /* Copy 9th bit to TXB8 */
    UCSR0B &= ~(1<<TXB8);
    if (data & 0x0100){
      UCSR0B |= (1<<TXB8);
    }
  }
  /* Put data into buffer, sends the data */
  UDR0 = (uint8_t) data;
}

uint8_t 
uart_receive(void){
  /* Wait for data to be received */
  while (!(UCSR0A & (1<<RXC0)));

  uint8_t status = UCSR0A;
  uint8_t data   = UDR0;

  uart_last_error = status & ((1<<FE0) | (1<<DOR0) | (1<<UPE0));

  return data;
}

uint16_t 
uart_receive_9bit(void){

  while (!(UCSR0A & (1<<RXC0)));

  uint8_t status = UCSR0A;  // 
  uint8_t resh   = UCSR0B;
  uint8_t resl   = UDR0;

  uart_last_error = status & ((1<<FE0) | (1<<DOR0) | (1<<UPE0));
  /* Filter the 9th bit, then return */

  return (uint16_t) (((resh >> RXB80 )&1) << 8) | resl;
}

void uart_flush(void) {
    uint8_t dummy;
    while (UCSR0A & (1 << RXC0)) {
        dummy = UDR0;  // auslesen und wegwerfen
    }
    (void)dummy;  // suppress unused warning
}

uint8_t
uart_data_available(void){
  return (UCSR0A & (1 << RXC0)) != 0;
}


uint8_t
uart_get_error(void){
  return uart_last_error;
}

void
uart_clear_error(void){
  uart_last_error=0;
}

void uart_print(const char str){
  while(*str){
    uart_transmit((uint8_t)*str++);
  }
}