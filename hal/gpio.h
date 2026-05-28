#ifndef GPIO_H_
#define GPIO_H_

#include <avr/io.h>
#include <inttypes.h>

#define HIGH 0x1
#define LOW 0x0

typedef enum GPIO_pin {
    Px0 = 0,
    Px1 = 1,
    Px2 = 2,
    Px3 = 3,
    Px4 = 4,
    Px5 = 5,
    Px6 = 6,
    Px7 = 7
} gpio_pin_t;

typedef enum GPIO_port {
    B = 0,
    C = 1,
    D = 2
} gpio_port_t;

// configuration
void GPIO_set_input(gpio_port_t port, gpio_pin_t pin);
void GPIO_set_output(gpio_port_t port, gpio_pin_t pin);
void GPIO_set_pullup(gpio_port_t port, gpio_pin_t pin);

// writing pins
void GPIO_set_pin_high(gpio_port_t port, gpio_pin_t pin);
void GPIO_set_pin_low(gpio_port_t port, gpio_pin_t pin);
void GPIO_toggle_pin(gpio_port_t port, gpio_pin_t pin);

// reading pins
uint8_t GPIO_read_pin(gpio_port_t port, gpio_pin_t pin);

#endif /* GPIO_H_ */
