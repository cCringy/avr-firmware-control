#include "debounce.h"

uint8_t debounce(gpio_port_t port, gpio_pin_t pin)
{
    static uint16_t state = 0;  // current debounce status
    state = (state << 1) | !GPIO_read_pin(port, pin) | 0xE000;  // 111X XXXX XXXX XXX1
    return (state == 0xF000) ? LOW : HIGH;                      // 1111 1111 0000 0000
}
