#ifndef TIMER_H
#define TIMER_H

#include "status.h"
#include <stdint.h>

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

typedef void (*timer_callback_t)(void);

typedef enum:uint8_t{ 
  HAL_TIMER_0, 
  HAL_TIMER_1, 
  HAL_TIMER_2,
  HAL_TIMER_COUNT
} timer_id_t;

typedef enum:uint8_t{ 
  TIMER_CH_A,
  TIMER_CH_B
} timer_channel_t;

typedef enum : uint8_t {
  TIMER_COM_DISCONNECTED,   // 00: Pin normaler GPIO
  TIMER_COM_TOGGLE,         // 01: Toggle on Compare Match
  TIMER_COM_CLEAR,          // 10: Non-inverting PWM / Clear on Match
  TIMER_COM_SET             // 11: Inverting PWM / Set on Match
} timer_com_t;

typedef enum : uint8_t {
  TIMER_IRQ_OVF,            // TOIEn,  Bit 0
  TIMER_IRQ_COMPA,          // OCIEnA, Bit 1
  TIMER_IRQ_COMPB,          // OCIEnB, Bit 2
  TIMER_IRQ_CAPT,           // ICIE1,  Bit 5 (nur Timer1)
  TIMER_IRQ_COUNT
} timer_irq_t;

typedef enum : uint8_t {
  TIMER_CLK_EXT_FALLING,    // CS = 110, Pin T0/T1
  TIMER_CLK_EXT_RISING      // CS = 111
} timer_ext_clk_t;

typedef enum:uint8_t{
    TIMER_MODE_NORMAL,
    TIMER_MODE_CTC,
    TIMER_MODE_FAST_PWM,
    TIMER_MODE_PHASE_CORRECT_PWM,
    TIMER_MODE_PHASE_FREQ_CORRECT_PWM,
    TIMER_MODE_COUNT
} timer_mode_t;

typedef enum:uint8_t{
  TIMER_TOP_OCRA,
  TIMER_TOP_ICR1,
  TIMER_TOP_0X00FF,
  TIMER_TOP_0X01FF,
  TIMER_TOP_0X03FF,
  TIMER_TOP_MAX,
  TIMER_TOP_COUNT
}timer_top_t;


[[nodiscard]] status_t timer_init(timer_id_t);// PRR-Bit löschen, Register auf Ausgangszustand, reservieren
[[nodiscard]] status_t timer_deinit(timer_id_t);

[[nodiscard]] status_t timer_start(timer_id_t ,uint16_t prescaler);
[[nodiscard]] status_t timer_stop(timer_id_t);

[[nodiscard]] status_t timer_set_channel(timer_id_t, timer_channel_t,timer_com_t);
[[nodiscard]] status_t timer_set_mode(timer_id_t,timer_mode_t,timer_top_t);

#endif