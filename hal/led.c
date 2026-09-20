#include "led.h"

static gpio_port_t led_port;
static gpio_pin_t led_pin;

status_t LED_init(gpio_port_t port, gpio_pin_t pin)
{
  led_port = port;
  led_pin = pin;
  return GPIO_set_output(led_port, led_pin);
}
status_t LED_on(void)
{
  return GPIO_set_pin_high(led_port, led_pin);
}
status_t LED_off(void)
{
  return GPIO_set_pin_low(led_port, led_pin);
}
status_t LED_toggle(void)
{
  return GPIO_toggle_pin(led_port, led_pin);
}
