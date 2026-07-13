#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <stdint.h>
#include "uart.h"

/*
Static für Static inline, d.h. jede TU kriegt seine eigene unabhängige Kopie von config
Konstant um Race Conditions vorzubeugen, da Module nur von Config lesen
Und sorgt für bessere Kapselung
*/

typedef struct {
  uint32_t    uart_baud;
  uart_mode_t uart_mode;
  uint8_t     async_double_speed;    
} config_t;

const config_t* board_config(void);

#endif // BOARD_CONFIG_H