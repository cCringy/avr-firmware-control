#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <stdint.h>
#include <avr/io.h>
#include "uart.h"
// F_CPU kommt über -DF_CPU aus dem Makefile, nicht nochmal hier definieren
// Default Baudrate is conservative Indusrty Standard
// slow enough for bad cables and fast enough for this usecase
#define BAUDRATE 9600UL  /*UL because if normal int on Arduino it will use 16 bit arithmetic
                         causing integer overflow*/
#define XCK PD4
                         /*
Static für Static inline, d.h. jede TU kriegt seine eigene unabhängige Kopie von config
Konstant um Race Conditions vorzubeugen, da Module nur von Config lesen
Und sorgt für bessere Kapselung
*/

typedef struct {
    uart_config_t uart;
} config_t;

const config_t * get_config(void);
#endif // BOARD_CONFIG_H