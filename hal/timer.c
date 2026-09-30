#include "timer.h"
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

typedef struct{timer_mode_t wave;timer_top_t top;uint8_t wgm;}wgm_entry_t;
typedef struct{uint16_t divisor;uint8_t cs;}prescaler_t;

// Timer0 and Timer2 share this table
static const wgm_entry_t timer2_modes[] = {
  { TIMER_MODE_NORMAL,             TIMER_TOP_MAX,  0 },
  { TIMER_MODE_PHASE_CORRECT_PWM,  TIMER_TOP_8BIT, 1 },
  { TIMER_MODE_CTC,                TIMER_TOP_OCRA, 2 },
  { TIMER_MODE_FAST_PWM,           TIMER_TOP_8BIT, 3 },
  { TIMER_MODE_PHASE_CORRECT_PWM,  TIMER_TOP_OCRA, 5 },
  { TIMER_MODE_FAST_PWM,           TIMER_TOP_OCRA, 7 },
};

static const wgm_entry_t t1_modes[] = {
  { TIMER_MODE_NORMAL,                 TIMER_TOP_MAX,   0 },
  { TIMER_MODE_PHASE_CORRECT_PWM,      TIMER_TOP_8BIT,  1 },
  { TIMER_MODE_PHASE_CORRECT_PWM,      TIMER_TOP_9BIT,  2 },
  { TIMER_MODE_PHASE_CORRECT_PWM,      TIMER_TOP_10BIT, 3 },
  { TIMER_MODE_CTC,                    TIMER_TOP_OCRA,  4 },
  { TIMER_MODE_FAST_PWM,               TIMER_TOP_8BIT,  5 },
  { TIMER_MODE_FAST_PWM,               TIMER_TOP_9BIT,  6 },
  { TIMER_MODE_FAST_PWM,               TIMER_TOP_10BIT, 7 },
  { TIMER_MODE_PHASE_FREQ_CORRECT_PWM, TIMER_TOP_ICR,   8 },
  { TIMER_MODE_PHASE_FREQ_CORRECT_PWM, TIMER_TOP_OCRA,  9 },
  { TIMER_MODE_PHASE_CORRECT_PWM,      TIMER_TOP_ICR,  10 },
  { TIMER_MODE_PHASE_CORRECT_PWM,      TIMER_TOP_OCRA, 11 },
  { TIMER_MODE_CTC,                    TIMER_TOP_ICR,  12 },
  { TIMER_MODE_FAST_PWM,               TIMER_TOP_ICR,  14 },
  { TIMER_MODE_FAST_PWM,               TIMER_TOP_OCRA, 15 },
};


static uint16_t configure_pre_and_return_top(uint16_t milliseconds);

static void (*timer_callback)(uint16_t) = 0;

void timer_handle_compare(uint16_t compare_value){
  timer_callback(compare_value);
}

void timer_set_interrupt_callback(void (*isr)(uint16_t)){
  timer_callback = isr;
}

status_t timer_init_timer(timer_id_t timer){
    TCCR1A = 0; // Lösche potentielle Voreinstellungen (e.g. PWM etc)
    TCCR1B = 0; // Lösche potentielle Voreinstellungen (e.g. PWM etc)

    timer_set_mode(timer, TIMER_MODE_NORMAL);
    return STATUS_OK;
}


static uint16_t configure_pre_and_return_top(uint16_t milliseconds){
    //gegeben
    const uint16_t timer_max=65535; //2^16-1
    const uint16_t pre_max = 1024;
    const uint16_t max_period_ms = (((uint32_t)timer_max * pre_max) / F_CPU)*1000;
    float timerFreq = (milliseconds > max_period_ms) ? max_period_ms : milliseconds;
    timerFreq = 1000.0f / timerFreq;
    //top = f_cpu / (pre * timerFreq)

    uint16_t top = 0;
    uint16_t pre = 0;

    if(round(F_CPU / ((pre=1)*timerFreq)) <= timer_max){

        TCCR1B |= (1 << CS10);

    }else if(round(F_CPU / ((pre=8)*timerFreq)) <= timer_max){

        TCCR1B |= (1<< CS11);

    }else if(round(F_CPU / ((pre=64)*timerFreq)) <= timer_max){

        TCCR1B |= (1<< CS11) | (1<<CS10);

    }else if(round(F_CPU / ((pre=256)*timerFreq)) <= timer_max){

        TCCR1B |= (1<< CS12);

    }else{

        pre = 1024;

        TCCR1B |= (1<< CS12) | (1<<CS10);
    }

    top = (uint16_t)round(F_CPU / ((float)pre * timerFreq)) - 1;

    return top;
}

void timer_init_timer1_pwm(){
    cli();
    TCCR1A = 0; // Lösche potentielle Voreinstellungen (e.g. PWM etc)
    TCCR1B = 0; // Lösche potentielle Voreinstellungen (e.g. PWM etc)
    
    TCCR1A |= (1<<WGM11) | (1<<WGM10);
    TCCR1B |= (1<<WGM12) | (1<<CS11) | (1<<CS10);//  FastPWM10bit;Prescaler, da LED 244 hz gut also pre=64
    
    // PWM Output auf OC1A aktivieren (PIN PB1 bei ATmega328)
    TCCR1A |= (1<<COM1A1); // Non-inverting mode

    OCR1A = 1023;
    sei();
}

uint16_t timer_fetch_comp(){
  return 0;
}
/*
void setTopValue(uint16_t top){
    OCR1AL = top & 0xFF;
    OCR1AH = top >> 8;
}
*/
ISR(TIMER1_COMPA_vect){

}