#include "led.h"

static gpio_id_t led;

status_t LED_init(gpio_id_t id){
  led = id;
  return GPIO_set_output(id);
}
status_t LED_on(void)
{
  return GPIO_set_pin_high(led);
}
status_t LED_off(void)
{
  return GPIO_set_pin_low(led);
}
status_t LED_toggle(void)
{
  return GPIO_toggle_pin(led);
}
