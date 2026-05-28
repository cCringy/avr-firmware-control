#ifndef ADC_H
#define ADC_H

#include <stdint.h>

void ADC_init(void);
void ADC_select_channel(uint8_t channel);
uint16_t ADC_poll(uint8_t channel);
void ADC_enable_interrupt(void);
void ADC_disable_interrupt(void);
void ADC_set_callback(void (func)(uint16_t));
void ADC_handle_Interrupt(void);
void ADC_start_conversion(void);
uint16_t ADC_fetch_conversion(void);
#endif