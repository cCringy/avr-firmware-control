#include "gpio.h"

void GPIO_set_input(gpio_port_t port, gpio_pin_t pin)
{
    switch (port)
    {
    case B:
        DDRB &= ~(1 << pin);
        break;
    case C:
        DDRC &= ~(1 << pin);
        break;
    case D:
        DDRD &= ~(1 << pin);
        break;
    default:
        break;
    }
}

void GPIO_set_output(gpio_port_t port, gpio_pin_t pin)
{
    switch (port)
    {
    case B:
        DDRB |= (1 << pin);
        break;
    case C:
        DDRC |= (1 << pin);
        break;
    case D:
        DDRD |= (1 << pin);
        break;
    default:
        break;
    }
}

void GPIO_set_pullup(gpio_port_t port, gpio_pin_t pin)
{
    switch (port)
    {
    case B:
        DDRB &= ~(1 << pin);
        PORTB |= (1 << pin);
        break;
    case C:
        DDRC &= ~(1 << pin);
        PORTC |= (1 << pin);
        break;
    case D:
        DDRD &= ~(1 << pin);
        PORTD |= (1 << pin);
        break;
    default:
        break;
    }
}

void GPIO_set_pin_high(gpio_port_t port, gpio_pin_t pin)
{
    switch (port)
    {
    case B:
        PORTB |= (1 << pin);
        break;
    case C:
        PORTC |= (1 << pin);
        break;
    case D:
        PORTD |= (1 << pin);
        break;
    default:
        break;
    }
}

void GPIO_set_pin_low(gpio_port_t port, gpio_pin_t pin)
{
    switch (port)
    {
    case B:
        PORTB &= ~(1 << pin);
        break;
    case C:
        PORTC &= ~(1 << pin);
        break;
    case D:
        PORTD &= ~(1 << pin);
        break;
    default:
        break;
    }
}

void GPIO_toggle_pin(gpio_port_t port, gpio_pin_t pin)
{
    switch (port)
    {
    case B:
        PORTB ^= (1 << pin);
        break;
    case C:
        PORTC ^= (1 << pin);
        break;
    case D:
        PORTD ^= (1 << pin);
        break;
    default:
        break;
    }
}

uint8_t GPIO_read_pin(gpio_port_t port, gpio_pin_t pin)
{
    switch (port)
    {
    case B:
        return (PINB & (1 << pin))?HIGH:LOW;
    case C:
        return (PINC & (1 << pin))?HIGH:LOW;
    case D:
        return (PIND & (1 << pin))?HIGH:LOW;
    default:
        return -1;
    }
}
