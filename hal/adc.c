#include "adc.h"
#include <avr/interrupt.h>
#include "debug_uart.h"

static FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar,_FDEV_SETUP_RW);
static void (*adc_callback)(uint16_t) = 0;

inline void ADC_select_channel(uint8_t channel){
    ADMUX &= 0b11110000; // delete prev channel selection
    ADMUX |= channel;
}

inline void ADC_start_conversion(void){
    ADCSRA |= (1 << ADSC);
}

inline uint16_t ADC_fetch_conversion(void){
    /*ADC liefert 10bit Ergebnis aber Register haben nur 8 bit. Deswegen ist das Ergebnis in 2 Register so aufgeteilt:
    ADCL mit den unteren 8 bit
    ADCH mit den oberen 2 bit
    */
    return ADCL/*unteren 8 bit und mus nach datenblatt zuerst gelesen werden weil sonst 'alter ADC' wert*/ | (ADCH<<8)/*oberen 2 Bit von ADCH*/;
}

void ADC_init(void){
    ADMUX |= (1<<REFS0);

    ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0); // Prescaler

    ADCSRA |= (1 << ADEN);
}


uint16_t ADC_poll(uint8_t channel){
    ADC_select_channel(channel);
    ADC_start_conversion();

    while (ADCSRA & (1<<ADSC)){/*wait for conversion to end and do nothing*/}

    return ADC_fetch_conversion();
}

void ADC_enable_interrupt(void){
    cli();
    ADCSRA |= (1<<ADIE);
    sei();
}

inline void ADC_disable_interrupt(void)
{
    ADCSRA &= ~(1 << ADIE);
}

void ADC_set_callback(void (func)(uint16_t)){
    adc_callback = func;
}

void ADC_handle_Interrupt(void){
    if (adc_callback!=0){
        uint16_t adcval = ADC_fetch_conversion();
        adc_callback(adcval);
    }
}