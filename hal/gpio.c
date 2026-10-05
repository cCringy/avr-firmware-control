#include "gpio.h"
#include <stdint.h>
#include <avr/io.h>
#include "status.h"

status_t GPIO_set_input(gpio_id_t id){
  gpio_port_t port = id.port;
  gpio_pin_t  pin = id.pin;
  switch (port){
    case GPIO_PORT_B:
      DDRB &= (uint8_t)~(1 << pin);
      break;
    case GPIO_PORT_C:
      DDRC &= (uint8_t)~(1 << pin);
      break;
    case GPIO_PORT_D:
      DDRD &= (uint8_t)~(1 << pin);
      break;
    default:
      return STATUS_ERR_PARAM;
  }
  return STATUS_OK;
}

status_t GPIO_set_output(gpio_id_t id){
  gpio_port_t port = id.port;
  gpio_pin_t pin = id.pin;
  switch (port){
    case GPIO_PORT_B:
      DDRB |= (uint8_t)(1 << pin);
      break;
    case GPIO_PORT_C:
      DDRC |= (uint8_t)(1 << pin);
      break;
    case GPIO_PORT_D:
      DDRD |= (uint8_t)(1 << pin);
      break;
    default:
      return STATUS_ERR_PARAM;
  }
  return STATUS_OK;
}

status_t GPIO_set_pullup(gpio_id_t id){
  gpio_port_t port = id.port;
  gpio_pin_t pin = id.pin;
  switch (port){
    case GPIO_PORT_B:
      DDRB  &= (uint8_t)~(1 << pin);
      PORTB |= (uint8_t) (1 << pin);
      break;
    case GPIO_PORT_C:
      DDRC  &= (uint8_t)~(1 << pin);
      PORTC |= (uint8_t) (1 << pin);
      break;
    case GPIO_PORT_D:
      DDRD  &= (uint8_t)~(1 << pin);
      PORTD |= (uint8_t) (1 << pin);
      break;
    default:
      return STATUS_ERR_PARAM;
  }
  return STATUS_OK;
}

status_t GPIO_set_pin_high(gpio_id_t id){
  gpio_port_t port = id.port;
  gpio_pin_t pin = id.pin;

  switch (port){
    case GPIO_PORT_B:
      PORTB |= (uint8_t)(1 << pin);
      break;
    case GPIO_PORT_C:
      PORTC |= (uint8_t)(1 << pin);
      break;
    case GPIO_PORT_D:
      PORTD |= (uint8_t)(1 << pin);
      break;
    default:
      return STATUS_ERR_PARAM;
  }
  return STATUS_OK;
}

status_t GPIO_set_pin_low(gpio_id_t id){
  gpio_port_t port = id.port;
  gpio_pin_t pin = id.pin;
  switch (port){
    case GPIO_PORT_B:
        PORTB &= (uint8_t)~(1 << pin);
        break;
    case GPIO_PORT_C:
        PORTC &= (uint8_t)~(1 << pin);
        break;
    case GPIO_PORT_D:
        PORTD &= (uint8_t)~(1 << pin);
        break;
    default:
        return STATUS_ERR_PARAM;
    }
    return STATUS_OK;
}

status_t GPIO_toggle_pin(gpio_id_t id){
  gpio_port_t port = id.port;
  gpio_pin_t pin = id.pin;
  switch (port){
    case GPIO_PORT_B:
      PORTB ^= (uint8_t)(1 << pin);
      break;
    case GPIO_PORT_C:
      PORTC ^= (uint8_t)(1 << pin);
      break;
    case GPIO_PORT_D:
     PORTD ^= (uint8_t)(1 << pin);
      break;
    default:
      return STATUS_ERR_PARAM;
  }
  return STATUS_OK;
}

status_t GPIO_read_pin(gpio_id_t id, uint8_t *value){
  gpio_port_t port = id.port;
  gpio_pin_t pin = id.pin;

  switch (port){
    case GPIO_PORT_B:
      *value = (uint8_t)(PINB & (1 << pin)) ? GPIO_HIGH : GPIO_LOW;
      break;
    case GPIO_PORT_C:
      *value = (uint8_t)(PINC & (1 << pin)) ? GPIO_HIGH : GPIO_LOW;
      break;
    case GPIO_PORT_D:
      *value = (uint8_t)(PIND & (1 << pin)) ? GPIO_HIGH : GPIO_LOW;
      break;
    default:
      return STATUS_ERR_PARAM;
  }
  return STATUS_OK;
}
