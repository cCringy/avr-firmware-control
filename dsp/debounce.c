#include "debounce.h"

uint8_t debounce(gpio_port_t port, gpio_pin_t pin)
{
    static uint16_t state = 0;  // current debounce status
    uint8_t pin_state = LOW;
    GPIO_read_pin(port, pin, &pin_state);
    state = (state << 1) | !pin_state | 0xE000;  // 111X XXXX XXXX XXX1
    return (state == 0xF000) ? LOW : HIGH;        // 1111 1111 0000 0000
}
