#include "gpio.h"

status_t GPIO_set_input(gpio_port_t port, gpio_pin_t pin)
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
        return STATUS_ERR_PARAM;
    }
    return STATUS_OK;
}

status_t GPIO_set_output(gpio_port_t port, gpio_pin_t pin)
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
        return STATUS_ERR_PARAM;
    }
    return STATUS_OK;
}

status_t GPIO_set_pullup(gpio_port_t port, gpio_pin_t pin)
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
        return STATUS_ERR_PARAM;
    }
    return STATUS_OK;
}

status_t GPIO_set_pin_high(gpio_port_t port, gpio_pin_t pin)
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
        return STATUS_ERR_PARAM;
    }
    return STATUS_OK;
}

status_t GPIO_set_pin_low(gpio_port_t port, gpio_pin_t pin)
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
        return STATUS_ERR_PARAM;
    }
    return STATUS_OK;
}

status_t GPIO_toggle_pin(gpio_port_t port, gpio_pin_t pin)
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
        return STATUS_ERR_PARAM;
    }
    return STATUS_OK;
}

status_t GPIO_read_pin(gpio_port_t port, gpio_pin_t pin, uint8_t *value)
{
    switch (port)
    {
    case B:
        *value = (PINB & (1 << pin)) ? HIGH : LOW;
        break;
    case C:
        *value = (PINC & (1 << pin)) ? HIGH : LOW;
        break;
    case D:
        *value = (PIND & (1 << pin)) ? HIGH : LOW;
        break;
    default:
        return STATUS_ERR_PARAM;
    }
    return STATUS_OK;
}
