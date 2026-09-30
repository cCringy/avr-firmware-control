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

typedef enum : uint8_t { TIMER_0, TIMER_1, TIMER_2, TIMER_NUM } timer_id_t;
typedef enum : uint8_t { TIMER_CH_A, TIMER_CH_B } timer_channel_t;

typedef enum : uint8_t {
    TIMER_MODE_NORMAL, TIMER_MODE_CTC, TIMER_MODE_FAST_PWM,
    TIMER_MODE_PHASE_CORRECT_PWM, TIMER_MODE_PHASE_FREQ_CORRECT_PWM,
    TIMER_MODE_NUM
} timer_mode_t;

typedef enum : uint8_t {
    TIMER_TOP_MAX, TIMER_TOP_8BIT, TIMER_TOP_9BIT, TIMER_TOP_10BIT,
    TIMER_TOP_OCRA, TIMER_TOP_ICR, TIMER_TOP_NUM
} timer_top_t;

typedef enum : uint8_t {
    TIMER_CLK_STOP, TIMER_CLK_DIV1, TIMER_CLK_DIV8, TIMER_CLK_DIV32, TIMER_CLK_DIV64,
    TIMER_CLK_DIV128, TIMER_CLK_DIV256, TIMER_CLK_DIV1024,
    TIMER_CLK_EXT_FALLING, TIMER_CLK_EXT_RISING
} timer_clock_t;

typedef enum : uint8_t { TIMER_OUT_OFF, TIMER_OUT_TOGGLE, TIMER_OUT_PWM, TIMER_OUT_PWM_INVERTED } timer_out_t;
typedef enum : uint8_t { TIMER_EVT_OVERFLOW, TIMER_EVT_COMPARE_A, TIMER_EVT_COMPARE_B, TIMER_EVT_CAPTURE } timer_event_t;

typedef void (*timer_cb_t)(void);

typedef struct {
    uint8_t      bits;       // 8 oder 16
    bool         running;    // CS != 0
    bool         reserved;
    timer_mode_t mode;
    timer_top_t  top;
    uint32_t     tick_hz;    // 0 wenn gestoppt oder externer Takt
} timer_info_t;

typedef struct { uint8_t min_bits; timer_mode_t mode; } timer_req_t;


typedef void (*timer_cb_t)(void);   // läuft im ISR-Kontext

// Ebene 2: was der Nutzer will (kann nur gültige Kombinationen erzeugen)
status_t timer_start_periodic_us(timer_id_t, uint32_t period_us, timer_cb_t);
status_t timer_pwm_start(timer_id_t, timer_channel_t, uint32_t freq_hz);
status_t timer_pwm_set_duty(timer_id_t, timer_channel_t, uint16_t permille);
status_t timer_pwm_set_pulse_us(timer_id_t, timer_channel_t, uint16_t us);
status_t timer_stop(timer_id_t);

// Ebene 1: 
status_t timer_init(timer_id_t);        // PRR-Bit löschen, Register auf Ausgangszustand, reservieren
status_t timer_deinit(timer_id_t);
status_t timer_set_mode(timer_id_t, timer_mode_t);
status_t timer_set_top_source(timer_id_t, timer_top_t);
status_t timer_set_top_value(timer_id_t, uint16_t, bool keep_count);
status_t timer_set_compare(timer_id_t, timer_channel_t, uint16_t);
status_t timer_set_output(timer_id_t, timer_channel_t, timer_out_t);
status_t timer_set_clock(timer_id_t, timer_clock_t);      // Start/Stop
status_t timer_set_callback(timer_id_t, timer_event_t, timer_cb_t);  // NULL = Interrupt aus

// Introspektion (Daten, kein Text)
status_t timer_get_info(timer_id_t, timer_info_t *);
status_t timer_find(const timer_req_t *, timer_id_t *);   // optional, für Laien
#endif