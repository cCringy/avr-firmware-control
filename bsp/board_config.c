#include "board_config.h"


static const config_t cfg = {
    .uart = {
        .baudrate = BAUDRATE,
        .mode = UART_MODE_ASYNC,
        .framesize = UART_FRAME_SIZE_8,
        .parity = UART_PARITY_DISABLED,
        .two_stop_bits = 0,
        .async_double_speed = 0,
    }
};

const config_t * get_config(void){
  return &cfg;
}