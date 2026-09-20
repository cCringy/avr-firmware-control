#ifndef ADC_H
#define ADC_H

#include <stdint.h>
#include "status.h"

typedef enum Channel{
  ADC0 = 0b0000,
  ADC1 = 0b0001,
  ADC2 = 0b0010,
  ADC3 = 0b0011,
  ADC4 = 0b0100,
  ADC5 = 0b0101,
  ADC6 = 0b0110,
  ADC7 = 0b0111,
  ADC8 = 0b1000,
  ONEV = 0b1110,
  GND  = 0b1111
}adc_channel_t;

void ADC_init(void);
void ADC_select_channel(adc_channel_t channel);
status_t ADC_poll(adc_channel_t channel, uint16_t *result);
void ADC_enable_interrupt(void);
void ADC_disable_interrupt(void);
void ADC_set_callback(void (func)(uint16_t));
void ADC_handle_Interrupt(void);
void ADC_start_conversion(void);
uint16_t ADC_fetch_conversion(void);
#endif