#include <stdlib.h>
#include "buffer.h"
#include "filter.h"

uint16_t filter_moving_average(buffer_t buffer, uint16_t num_taps){
    uint32_t acc = 0;
    for(int i = 0; i<num_taps;++i){
        acc += buffer_peek(buffer,i);
    }
    return (uint16_t) (acc / num_taps);
}

uint16_t filter_sound_sensor(buffer_t buffer, uint16_t num_taps){
    uint32_t acc = 0;
    for(int i = 0; i<num_taps;++i){
        uint16_t val = (uint16_t)buffer_peek(buffer,i)-337;//  DC  Bias entfernen 
        acc += (uint32_t)val*val;
    }
    return (uint16_t) (acc / num_taps);
}