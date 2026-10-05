#ifndef GPIO_H_
#define GPIO_H_

#include <stdint.h>
#include "status.h"

#define GPIO_HIGH 0x1
#define GPIO_LOW 0x0

typedef enum GPIO_pin : uint8_t{
    GPIO_PIN_Px0 = 0,
    GPIO_PIN_Px1 = 1,
    GPIO_PIN_Px2 = 2,
    GPIO_PIN_Px3 = 3,
    GPIO_PIN_Px4 = 4,
    GPIO_PIN_Px5 = 5,
    GPIO_PIN_Px6 = 6,
    GPIO_PIN_Px7 = 7
} gpio_pin_t;

typedef enum GPIO_port : uint8_t{
    GPIO_PORT_B = 0,
    GPIO_PORT_C = 1,
    GPIO_PORT_D = 2
} gpio_port_t;

typedef struct GPIOid{
  gpio_port_t port;
  gpio_pin_t  pin;
}gpio_id_t;

// configuration
status_t GPIO_set_input(gpio_id_t);
status_t GPIO_set_output(gpio_id_t);
status_t GPIO_set_pullup(gpio_id_t);

// writing pins
status_t GPIO_set_pin_high(gpio_id_t);
status_t GPIO_set_pin_low(gpio_id_t);
status_t GPIO_toggle_pin(gpio_id_t);

// reading pins
status_t GPIO_read_pin(gpio_id_t, uint8_t *value);

#endif /* GPIO_H_ */
