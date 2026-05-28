#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "drivers/adc.h"
#include "debug_uart.h"
#include "drivers/led.h"
#include "drivers/gpio.h"
#include "drivers/adc.h"
#include "dsp/buffer.h"
#include "helpers/debounce.h"
#include "dsp/buffer.h"
#include "board_config.h"

#define N_TAPS 10
#define BUFFER_SIZE 20

static FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar,_FDEV_SETUP_RW);
volatile buffer_t input_buffer;

void adc_interrupt_handler(uint16_t val){
    uint8_t msb=val >> 8;
    uint8_t lsb = val & 0xFF;
    uart_transmit(msb);
    uart_transmit(lsb);
}

void adc_interrupt_handler2(uint16_t val){
    buffer_insert(input_buffer, val);
}

void setup(void){
    stdin = stdout = &uart_str;
    uart_init();
    
    ADC_init();
    ADC_set_callback(&adc_interrupt_handler2);
    ADC_enable_interrupt();

    LED_init();
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
        LED_toggle();
        _delay_ms(500);
    }
}

ISR(ADC_vect){
    uint16_t adcval = ADC_fetch_conversion();
    //if(adc_callback != 0){
    //    adc_callback(adcval);
    //}
    buffer_insert(input_buffer, adcval);
    ADC_start_conversion();
}