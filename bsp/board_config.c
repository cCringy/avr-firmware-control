#include "board_config.h"

static const config_t cfg = {
    .uart_baud = 115200,
    .uart_mode = UART_ASYNC,
  
};

const config_t* board_config(void) { return &cfg; }