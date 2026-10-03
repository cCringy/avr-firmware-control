#include "timer.h"
#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#define WGM_RESERVED {TIMER_MODE_COUNT,TIMER_TOP_COUNT}

static void (*timer_callback)(void);

// Uses CS1x for both Timer1 and Timer2, this works because the bit positions are the same
static const uint16_t pre_val_01[] = { 1, 8, 64, 256, 1024 };
static const uint8_t  pre_bits_01[] = {
    (1 << CS10),                 // /1
    (1 << CS11),                 // /8
    (1 << CS11) | (1 << CS10),   // /64
    (1 << CS12),                 // /256
    (1 << CS12) | (1 << CS10),   // /1024
};

static const uint16_t pre_val_2[]  PROGMEM = { 1, 8, 32, 64, 128, 256, 1024 };
static const uint8_t  pre_bits_2[] PROGMEM= { (1<<CS20), (1<<CS21),
                                              (1<<CS21)|(1<<CS20),
                                              (1<<CS22),
                                              (1<<CS22)|(1<<CS20),
                                              (1<<CS22)|(1<<CS21), 
                                              (1<<CS22)|(1<<CS21)|(1<<CS20)
                                            };

typedef struct WaveformMode{
  timer_mode_t mode;
  timer_top_t  top_value;
}wgm_mode_t;

typedef struct TimerDescriptor {
  volatile uint8_t *tccra, *tccrb, *timsk, *tifr;
  uint8_t           cs_mask;
  uint16_t          top_max;      // UINT8_MAX or UINT16_MAX
  const uint16_t   *pre_val;
  const uint8_t    *pre_bits;
  uint8_t           pre_count;
  const wgm_mode_t *timer_mode_table;
  uint8_t           timer_mode_table_count;
} timer_desc_t;

 // modes for timer0 range from 0-7 
static const wgm_mode_t timer0[] PROGMEM = {
  {TIMER_MODE_NORMAL            ,TIMER_TOP_MAX     },
  {TIMER_MODE_PHASE_CORRECT_PWM ,TIMER_TOP_0X00FF  },
  {TIMER_MODE_CTC               ,TIMER_TOP_OCRA    },
  {TIMER_MODE_FAST_PWM          ,TIMER_TOP_0X00FF  },
  WGM_RESERVED,
  {TIMER_MODE_PHASE_CORRECT_PWM ,TIMER_TOP_OCRA    },
  WGM_RESERVED,
  {TIMER_MODE_FAST_PWM          ,TIMER_TOP_OCRA    }
};

static const wgm_mode_t timer1[] PROGMEM = {
  {TIMER_MODE_NORMAL                  ,TIMER_TOP_MAX     },
  {TIMER_MODE_PHASE_CORRECT_PWM       ,TIMER_TOP_0X00FF  },
  {TIMER_MODE_PHASE_CORRECT_PWM       ,TIMER_TOP_0X01FF  },
  {TIMER_MODE_PHASE_CORRECT_PWM       ,TIMER_TOP_0X03FF  },
  {TIMER_MODE_CTC                     ,TIMER_TOP_OCRA    },
  {TIMER_MODE_FAST_PWM                ,TIMER_TOP_0X00FF  },
  {TIMER_MODE_FAST_PWM                ,TIMER_TOP_0X01FF  },
  {TIMER_MODE_FAST_PWM                ,TIMER_TOP_0X03FF  },
  {TIMER_MODE_PHASE_FREQ_CORRECT_PWM  ,TIMER_TOP_ICR1    },
  {TIMER_MODE_PHASE_FREQ_CORRECT_PWM  ,TIMER_TOP_OCRA    },
  {TIMER_MODE_PHASE_CORRECT_PWM       ,TIMER_TOP_ICR1    },
  {TIMER_MODE_PHASE_CORRECT_PWM       ,TIMER_TOP_OCRA    },
  {TIMER_MODE_CTC                     ,TIMER_TOP_ICR1    },
  WGM_RESERVED,
  {TIMER_MODE_FAST_PWM                ,TIMER_TOP_ICR1    },
  {TIMER_MODE_FAST_PWM                ,TIMER_TOP_OCRA    }
};

static const wgm_mode_t timer2[] PROGMEM = {
  {TIMER_MODE_NORMAL            ,TIMER_TOP_MAX     },
  {TIMER_MODE_PHASE_CORRECT_PWM ,TIMER_TOP_0X00FF  },
  {TIMER_MODE_CTC               ,TIMER_TOP_OCRA    },
  {TIMER_MODE_FAST_PWM          ,TIMER_TOP_0X00FF  },
  WGM_RESERVED,
  {TIMER_MODE_PHASE_CORRECT_PWM ,TIMER_TOP_OCRA    },
  WGM_RESERVED,
  {TIMER_MODE_FAST_PWM          ,TIMER_TOP_OCRA    }
};

static const timer_desc_t timers[HAL_TIMER_COUNT] PROGMEM = {
  [HAL_TIMER_0] = { 
                    &TCCR0A,&TCCR0B,&TIMSK0,&TIFR0, 
                    (1<<CS02)|(1<<CS01)|(1<<CS00),
                    UINT8_MAX,  
                    pre_val_01, 
                    pre_bits_01, 
                    ARRAY_SIZE(pre_val_01),
                    timer0,
                    ARRAY_SIZE(timer0)
                  },
  [HAL_TIMER_1] = {
                    &TCCR1A,&TCCR1B,&TIMSK1,&TIFR1,
                    (1<<CS12)|(1<<CS11)|(1<<CS10),
                    UINT16_MAX,
                    pre_val_01,
                    pre_bits_01,
                    ARRAY_SIZE(pre_val_01),
                    timer1,
                    ARRAY_SIZE(timer1)
                  },
  [HAL_TIMER_2] = {
                    &TCCR2A,&TCCR2B,&TIMSK2,&TIFR2, 
                    (1<<CS22)|(1<<CS21)|(1<<CS20),
                    UINT8_MAX,
                    pre_val_2,
                    pre_bits_2,
                    ARRAY_SIZE(pre_val_2),
                    timer2,
                    ARRAY_SIZE(timer2)
                  },
};

status_t timer_init(timer_id_t t){
  if(HAL_TIMER_COUNT <= t) return STATUS_ERR_PARAM;
  *timers[t].tccrb &= ~timers[t].cs_mask;
  
  return STATUS_OK;
}

static status_t write_wgm(const timer_desc_t * d,uint8_t wgm){
  if(wgm>= d->timer_mode_table_count) return STATUS_ERR_PARAM;

  *d->tccra = (*d->tccra & (uint8_t)~0x03) | (wgm & (uint8_t)0x03);
  *d->tccrb = (*d->tccrb & (uint8_t)~0x18) | ((wgm & (uint8_t)0x0C) << 1);
  return STATUS_OK;
}

status_t timer_set_mode(timer_id_t t , timer_mode_t mode,timer_top_t top){
  if(HAL_TIMER_COUNT <= t || TIMER_MODE_COUNT <= mode || TIMER_TOP_COUNT<=top) return STATUS_ERR_PARAM;
  uint8_t wgm =  0;
  const timer_desc_t *  d = &timers[t];
  for(; wgm<d->timer_mode_table_count;wgm++){
    if(d->timer_mode_table[wgm].mode == mode &&d->timer_mode_table[wgm].top_value == top){
      return write_wgm(d,wgm);
    }
  }

  return  STATUS_ERR_PARAM;
}
/**
 * Picks the smallest prescaler for which the requested period fits into the
 * timer's counter, writes the CS bits (this starts the timer) and returns TOP.
 *
 * Formulas (CTC mode, compare-match interrupt once per period):
 *
 *   f_int    = F_CPU / (pre * (TOP + 1))          // datasheet, CTC mode
 *   T        = (pre * (TOP + 1)) / F_CPU          // period in seconds
 *   counts   = TOP + 1 = T * F_CPU / pre
 *
 * With T in milliseconds, avoiding float:
 *
 *   ticks_per_ms = F_CPU / 1000                   // timer ticks per ms at pre = 1
 *   counts       = ticks_per_ms * ms / pre
 *   TOP          = counts - 1
 *
 * Rounding to the nearest integer without float:
 *
 *   round(a / b) = (a + b/2) / b                  // for unsigned a, b
 *
 * Constraint: counts <= top_max + 1               // 256 (T0/T2) or 65536 (T1)
 *
 * Longest period (largest prescaler, full counter):
 *
 *   max_ms = (top_max + 1) * pre_max / ticks_per_ms
 *          = 65536 * 1024 / 16000 = 4194 ms (Timer1, 16 MHz)
 *          = 256   * 1024 / 16000 = 16 ms   (Timer0/2, 16 MHz)
 *
 * Magnitudes (uint32_t is enough, no overflow):
 *   ticks_per_ms * ms <= 16000 * 4194 = 67.1e6 < 4.29e9
 */
static status_t configure_pre_and_return_top(timer_id_t t,uint16_t ms, uint16_t * out_top){
  if(t>= HAL_TIMER_COUNT || !out_top){return STATUS_ERR_PARAM;}

  const timer_desc_t *d = &timers[t];

  const uint32_t ticks_per_ms = F_CPU/1000UL;
  const uint32_t max_counts   = (uint32_t)d->top_max + 1UL;
  const uint32_t max_ms       = (max_counts * d->pre_val[d->pre_count-1]) / ticks_per_ms;

  if(ms == 0)     ms = 1;
  if(ms > max_ms) ms = (uint16_t) max_ms;

  uint8_t i = 0;
  uint32_t counts = 0;

  for(; i < d->pre_count;i++){
    counts = (ticks_per_ms * ms + d->pre_val[i] / 2) / d->pre_val[i];
    if(counts <= max_counts) break;
  }
  if( i == d->pre_count) {i =  d->pre_count-1; counts = max_counts;}
  if(counts == 0) counts = 1;

  *d->tccrb = (*d->tccrb & (uint8_t)~d->cs_mask) | d->pre_bits[i];
  *out_top = (uint16_t)(counts-1);

  return STATUS_OK;
}

status_t timer_start(timer_id_t t, uint16_t prescaler){
  if(t >= HAL_TIMER_COUNT) return STATUS_ERR_PARAM;
  timer_desc_t * d = &timers[t];

  if(d->pre_val[d->pre_count] < prescaler){
    return STATUS_ERR_PARAM;
  }

  for(int i = 0 ; i < d->pre_count ; i++){
    if(d->pre_val[i] == prescaler){
      *d->tccrb &= d->pre_bits[i];
      return STATUS_OK;
    }
  }

  return STATUS_ERR_PARAM;
}
status_t timer_stop(t){
  if(t >= HAL_TIMER_COUNT) return STATUS_ERR_PARAM;

  timer_desc_t * d = &timers[t];
  *d->tccrb &= (uint8_t) ~d->cs_mask;

  return STATUS_OK;
}

ISR(TIMER1_COMPA_vect){

}