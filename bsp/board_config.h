#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <stdint.h>
#include <avr/io.h>
#include "uart.h"
#define F_CPU 16000000UL
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



const config_t* board_config(void);

#endif // BOARD_CONFIG_H