#ifndef DEBOUNCE_H_
#define DEBOUNCE_H_

#include "../drivers/gpio.h"

uint8_t debounce(gpio_port_t port, gpio_pin_t pin);

#endif /* DEBOUNCE_H_ */