#ifndef LED_H_
#define LED_H_

#include <avr/io.h>
#include "gpio.h"
#include "status.h"

status_t LED_init(gpio_id_t);
status_t LED_on(void);
status_t LED_off(void);
status_t LED_toggle(void);

#endif /* LED_H_ */