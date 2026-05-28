#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>

#ifndef F_CPU
#define F_CPU 16000000UL // 16 MHz
#endif

void timer_init_timer1(uint16_t milliseconds);
void timer_stop(void);

#endif