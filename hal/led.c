#include "led.h"

void LED_init()
{
    DDRB |= 1 << LED_PIN; // 0b00100000
}
void LED_on()
{
    PORTB |= 1 << LED_PIN;
}
void LED_off()
{
    PORTB &= ~(1 << LED_PIN);
}
void LED_toggle()
{
    PORTB ^= 1 << LED_PIN;
}