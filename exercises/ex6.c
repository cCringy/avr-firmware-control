#include <stdio.h>
#include <util/delay.h>

#include "board_config.h"
#include "debug_uart.h"
#include "drivers/led.h"
#include "drivers/gpio.h"
#include "drivers/adc.h"
#include "helpers/debounce.h"

static FILE uart_str =  FDEV_SETUP_STREAM(uart_putchar,uart_getchar,_FDEV_SETUP_RW);

void setup(void){
    stdout = stdin = &uart_str;
    //uart.init();
    printf("UART initialised\n");

    LED_init();
    printf("LED initialised\n");

    GPIO_set_pullup(PUSH_BUTTON_PORT, PUSH_BUTTON_PIN);/*Board config*/
    printf("Push-button initialised\n");

	ADC_init();
    printf("ADC initialised\n");
}

int main(void){
    setup();

    while(1){
        uint16_t adcval = ADC_poll(PHOTORESISTOR_CH);
        
        if(adcval<PHOTORESISTOR_THRESH){
            LED_on();
        }else{
            LED_off();

        }

        if(debounce(PUSH_BUTTON_PORT, PUSH_BUTTON_PIN) == LOW)
		{
	        printf("Photoresistor value: %u\n", adcval);
		}
    }

    return 0;
}