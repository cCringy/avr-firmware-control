#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>
#include "status.h"

#ifndef F_CPU
#define F_CPU 16000000UL // 16 MHz
#endif

// Clock Sources

// timer0 modes : normal , ctc , fast pwm, phase correct pwm

/*timer1 modes:
- normal
- Clear Timer on Compare Match (CTC) Mode
- Fast PWM Mode
- Phase Correct PWM Mode
-  Phase and Frequency Correct PWM Mode
*/  
// timer1 features Input Capture and Noice Canceler

// timer2 feature async clocking
/*
timer 2 modes:
- normal
- ctc
- fast pwm
- phase correct pwm
*/
typedef enum Timer{
  TIMER_COUNTER_0,
  TIMER_COUNTER_1,
  TIMER_COUNTER_2
}timer_t;

typedef enum TimerModes{
  NORMAL,
  CTC, 
  FAST_PWM,
  PHASE_CORRECT_PWM,
  PHASE_FREQ_CORRECT_PWM,
}timer_mode_t;

status_t timer_init_timer(timer_t timer);
status_t timer_stop(timer_t timer);
void timer_set_interruptfunction(void);

#endif