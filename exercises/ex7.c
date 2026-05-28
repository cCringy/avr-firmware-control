#include <stdio.h>
#include <util/delay.h>

#include "board_config.h"
#include "debug_uart.h"
#include "drivers/led.h"
#include "drivers/gpio.h"
#include "drivers/adc.h"
#include "dsp/buffer.h"
#include "helpers/debounce.h"

#define N_TAPS 10
#define BUFFER_SIZE 20

static FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar,_FDEV_SETUP_RW);

buffer_t input_buffer;
buffer_t output_buffer;

void setup(void){
    stdin = stdout = &uart_str;
    uart_init();
    printf("UART initialised\n");

    ADC_init();
    printf("ADC initialised\n");

    input_buffer = buffer_create(BUFFER_SIZE);
    output_buffer = buffer_create(BUFFER_SIZE);
}

int main(){

    setup();

    while(1){
        uint16_t adcval = ADC_poll(0);
        /* to make 10 tap average filter, we need to look 10 units into the past so we wait until buffer is full*/
        if(buffer_getlen(input_buffer)>N_TAPS){
            uint16_t filter_out = filter_moving_average(input_buffer, N_TAPS);
            buffer_insert(output_buffer, (item_t) filter_out);
        }

        item_t value = buffer_pop(output_buffer);
        
        if(value != -1){
            uint8_t msb = adcval >> 8;
            uint8_t lsb = adcval & 0xFF;
            uart_transmit(msb);
            uart_transmit(lsb);
        }
        _delay_ms(100);
    }

    buffer_delete(&input_buffer);
    buffer_delete(&output_buffer);

    return 0;
}