#include "gpio.h"
#include <stdint.h>
#include "status.h"

status_t GPIO_set_input(gpio_port_t port, gpio_pin_t pin)
{
    switch (port)
    {
    case B:
        DDRB &= (uint8_t)~(1 << pin);
        break;
    case C:
        DDRC &= (uint8_t)~(1 << pin);
        break;
    case D:
        DDRD &= (uint8_t)~(1 << pin);
        break;
    default:
        return STATUS_ERR_PARAM;
    }
    return STATUS_OK;
}

status_t GPIO_set_output(gpio_port_t port, gpio_pin_t pin)
{
    switch (port)
    {
    case B:
        DDRB |= (uint8_t)(1 << pin);
        break;
    case C:
        DDRC |= (uint8_t)(1 << pin);
        break;
    case D:
        DDRD |= (uint8_t)(1 << pin);
        break;
    default:
        return STATUS_ERR_PARAM;
    }
    return STATUS_OK;
}

status_t GPIO_set_pullup(gpio_port_t port, gpio_pin_t pin)
{
    switch (port)
    {
    case B:
        DDRB  &= (uint8_t)~(1 << pin);
        PORTB |= (uint8_t) (1 << pin);
        break;
    case C:
        DDRC  &= (uint8_t)~(1 << pin);
        PORTC |= (uint8_t) (1 << pin);
        break;
    case D:
        DDRD  &= (uint8_t)~(1 << pin);
        PORTD |= (uint8_t) (1 << pin);
        break;
    default:
        return STATUS_ERR_PARAM;
    }
    return STATUS_OK;
}

status_t GPIO_set_pin_high(gpio_port_t port, gpio_pin_t pin)
{
    switch (port)
    {
    case B:
        PORTB |= (uint8_t)(1 << pin);
        break;
    case C:
        PORTC |= (uint8_t)(1 << pin);
        break;
    case D:
        PORTD |= (uint8_t)(1 << pin);
        break;
    default:
        return STATUS_ERR_PARAM;
    }
    return STATUS_OK;
}

status_t GPIO_set_pin_low(gpio_port_t port, gpio_pin_t pin)
{
    switch (port)
    {
    case B:
        PORTB &= (uint8_t)~(1 << pin);
        break;
    case C:
        PORTC &= (uint8_t)~(1 << pin);
        break;
    case D:
        PORTD &= (uint8_t)~(1 << pin);
        break;
    default:
        return STATUS_ERR_PARAM;
    }
    return STATUS_OK;
}

status_t GPIO_toggle_pin(gpio_port_t port, gpio_pin_t pin)
{
    switch (port)
    {
    case B:
        PORTB ^= (uint8_t)(1 << pin);
        break;
    case C:
        PORTC ^= (uint8_t)(1 << pin);
        break;
    case D:
        PORTD ^= (uint8_t)(1 << pin);
        break;
    default:
        return STATUS_ERR_PARAM;
    }
    return STATUS_OK;
}

status_t GPIO_read_pin(gpio_port_t port, gpio_pin_t pin, uint8_t *value)
{
    switch (port)
    {
    case B:
        *value = (uint8_t)(PINB & (1 << pin)) ? HIGH : LOW;
        break;
    case C:
        *value = (uint8_t)(PINC & (1 << pin)) ? HIGH : LOW;
        break;
    case D:
        *value = (uint8_t)(PIND & (1 << pin)) ? HIGH : LOW;
        break;
    default:
        return STATUS_ERR_PARAM;
    }
    return STATUS_OK;
}
