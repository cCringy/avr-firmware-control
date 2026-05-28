#ifndef FILTER_H_
#define FILTER_H_
#include "buffer.h"
uint16_t filter_moving_average(buffer_t buffer, uint16_t num_taps);
#endif /* FILTER_H_ */