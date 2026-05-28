#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "drivers/timer.h"
#include "drivers/adc.h"
#include "debug_uart.h"
#include "dsp/buffer.h"
#include "board_config.h"

#define BUFFER_SIZE 20

static FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar,_FDEV_SETUP_RW);
volatile buffer_t input_buffer;

void insert_into_adcbuffer(uint16_t adcval){
    buffer_insert(input_buffer,adcval);
}

void setup(void){
    stdin = stdout = &uart_str;
    uart_init();
    
    ADC_init();
    ADC_select_channel(PHOTORESITOR_CH);
    ADC_set_callback(&insert_into_adcbuffer)
    ADC_enable_interrupt();

    timer_init();

    input_buffer = buffer_create(BUFFER_SIZE);
}

int main(void){
    setup();

    uint16_t val;

    while(1){
        if(buffer_getlen(input_buffer)!= 0){
            val = buffer_pop(input_buffer);
            uint8_t msb = val >> 8;
            uint8_t lsb = val & 0xFF;
            uart_transmit(msb);
            uart_transmit(lsb);
        }
    }
    cli();
    buffer_delete((buffer_t*) &input_buffer)
    return 0;
}

ISR(TIMER1_COMPA_vect){
    ADC_start_conversion();
}

ISR(ADC_vect){
    ADC_handle_Interrupt();
}

