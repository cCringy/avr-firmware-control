#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
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

typedef enum : uint8_t { 
  HAL_TIMER_0, 
  HAL_TIMER_1, 
  HAL_TIMER_2,
  HAL_TIMER_COUNT
} timer_id_t;

typedef enum : uint8_t { 
  TIMER_CH_A,
  TIMER_CH_B
} timer_channel_t;

typedef enum : uint8_t {
    TIMER_MODE_NORMAL,
    TIMER_MODE_CTC,
    TIMER_MODE_FAST_PWM,
    TIMER_MODE_PHASE_CORRECT_PWM,
    TIMER_MODE_PHASE_FREQ_CORRECT_PWM,
    TIMER_MODE_COUNT
} timer_mode_t;

typedef enum : uint8_t{
  TIMER_TOP_OCRA,
  TIMER_TOP_ICR1,
  TIMER_TOP_0X00FF,
  TIMER_TOP_0X01FF,
  TIMER_TOP_0X03FF,
  TIMER_TOP_MAX,
  TIMER_TOP_COUNT
}timer_top_t;


status_t timer_init(timer_id_t);// PRR-Bit löschen, Register auf Ausgangszustand, reservieren
status_t timer_set_channel(timer_id_t, timer_channel_t,timer_top_t);
status_t timer_set_mode(timer_id_t,timer_mode_t);

#endif