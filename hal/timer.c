#include "status.h"
#include "timer.h"
#include "power_intern.h"
#include "gpio.h"
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#if defined(__AVR__) && defined(__FLASH)
  #define HAL_FLASH __flash
#else
  #define HAL_FLASH
#endif

#ifndef F_CPU
#error "F_CPU must be defined by the build system (-DF_CPU=...)"
#endif

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#define WGM_RESERVED {TIMER_MODE_COUNT,TIMER_TOP_COUNT}

#define TIMER_CAP_16BIT    (1u << 0)
#define TIMER_CAP_ICR      (1u << 1)
#define TIMER_CAP_ASYNC    (1u << 2)
#define TIMER_CAP_EXT_CLK  (1u << 3)

//-----------------STRUCTS---------------------

typedef struct WaveformMode{
  timer_mode_t mode;
  timer_top_t  top_value;
}wgm_mode_t;

typedef struct TimerDescriptor {
  // Control / Interrupt
  volatile uint8_t           *tccra;
  volatile uint8_t           *tccrb;
  volatile uint8_t           *focr;     // FOCnA (Bit 7) / FOCnB (Bit 6): T0/T2 TCCRnB, T1 TCCR1C
  volatile uint8_t           *timsk;
  volatile uint8_t           *tifr;

  // Counter / Compare (16 Bit: Low-Byte, Access only via reg_write/reg_read)
  volatile uint8_t           *tcnt;
  volatile uint8_t           *ocra;
  volatile uint8_t           *ocrb;

  // Properties
  uint8_t                     caps;
  uint8_t                     prr_mask;
  uint8_t                     cs_mask;
  uint16_t                    top_max;  // UINT8_MAX oder UINT16_MAX

  // Prescaler
  const HAL_FLASH uint16_t   *pre_val;
  const HAL_FLASH uint8_t    *pre_bits;
  uint8_t                     pre_count;

  // WGM-Tabelle, Index = WGM-Wert
  const HAL_FLASH wgm_mode_t *timer_mode_table;
  uint8_t                     timer_mode_table_count;

  gpio_id_t oc_pin[2];
} timer_desc_t;

typedef struct {
  bool    in_use;
  bool    suspended;    // PRR-Bit gesetzt, Register nicht zugreifbar
  uint8_t wgm;          // aktuell gesetzter WGM-Index
  uint8_t prev_presc;   // zuletzt gewählter Prescaler-Index
} timer_state_t;

static_assert(sizeof(wgm_mode_t) == 2, "enum : uint8_t needs to work");

//---------------FILE GLOBALS---------------------

static timer_state_t state[HAL_TIMER_COUNT];

static volatile timer_callback_t cb;

//-----------------LOOKUP-TABLES---------------------
// Uses CS1x for both Timer0 and Timer1, this works because the bit positions are the same
static const HAL_FLASH uint16_t pre_val_01[]  = { 1, 8, 64, 256, 1024 };
static const HAL_FLASH uint8_t  pre_bits_01[] = {
    (1 << CS10),                 // /1
    (1 << CS11),                 // /8
    (1 << CS11) | (1 << CS10),   // /64
    (1 << CS12),                 // /256
    (1 << CS12) | (1 << CS10),   // /1024
};

static_assert(ARRAY_SIZE(pre_val_01) == ARRAY_SIZE(pre_bits_01));

static const HAL_FLASH uint16_t pre_val_2[]   = { 1, 8, 32, 64, 128, 256, 1024 };
static const HAL_FLASH uint8_t  pre_bits_2[]  = { (1<<CS20), (1<<CS21),
                                              (1<<CS21)|(1<<CS20),
                                              (1<<CS22),
                                              (1<<CS22)|(1<<CS20),
                                              (1<<CS22)|(1<<CS21), 
                                              (1<<CS22)|(1<<CS21)|(1<<CS20)
                                            };

static_assert(ARRAY_SIZE(pre_val_2) == ARRAY_SIZE(pre_bits_2));


 // modes for timer0 range from 0-7 
static const HAL_FLASH wgm_mode_t timer0[] = {
  {TIMER_MODE_NORMAL            ,TIMER_TOP_MAX     },
  {TIMER_MODE_PHASE_CORRECT_PWM ,TIMER_TOP_0X00FF  },
  {TIMER_MODE_CTC               ,TIMER_TOP_OCRA    },
  {TIMER_MODE_FAST_PWM          ,TIMER_TOP_0X00FF  },
  WGM_RESERVED,
  {TIMER_MODE_PHASE_CORRECT_PWM ,TIMER_TOP_OCRA    },
  WGM_RESERVED,
  {TIMER_MODE_FAST_PWM          ,TIMER_TOP_OCRA    }
};
static_assert(ARRAY_SIZE(timer0)==8);
static const HAL_FLASH wgm_mode_t timer1[] = {
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
static_assert(ARRAY_SIZE(timer1)==16);
static const HAL_FLASH wgm_mode_t timer2[] = {
  {TIMER_MODE_NORMAL            ,TIMER_TOP_MAX     },
  {TIMER_MODE_PHASE_CORRECT_PWM ,TIMER_TOP_0X00FF  },
  {TIMER_MODE_CTC               ,TIMER_TOP_OCRA    },
  {TIMER_MODE_FAST_PWM          ,TIMER_TOP_0X00FF  },
  WGM_RESERVED,
  {TIMER_MODE_PHASE_CORRECT_PWM ,TIMER_TOP_OCRA    },
  WGM_RESERVED,
  {TIMER_MODE_FAST_PWM          ,TIMER_TOP_OCRA    }
};
static_assert(ARRAY_SIZE(timer2)==8);

static const HAL_FLASH timer_desc_t timers[HAL_TIMER_COUNT] = {
  [HAL_TIMER_0] = {
    .tccra = &TCCR0A, .tccrb = &TCCR0B, .focr = &TCCR0B,
    .timsk = &TIMSK0, .tifr  = &TIFR0,
    .tcnt  = &TCNT0,  .ocra  = &OCR0A,  .ocrb = &OCR0B,

    .caps     = TIMER_CAP_EXT_CLK,
    .prr_mask = (1 << PRTIM0),
    .cs_mask  = (1 << CS02) | (1 << CS01) | (1 << CS00),
    .top_max  = UINT8_MAX,

    .pre_val   = pre_val_01,
    .pre_bits  = pre_bits_01,
    .pre_count = ARRAY_SIZE(pre_val_01),

    .timer_mode_table       = timer0,
    .timer_mode_table_count = ARRAY_SIZE(timer0),
    .oc_pin = {
      [TIMER_CH_A] = {GPIO_PORT_D,GPIO_PIN_Px6},
      [TIMER_CH_B] = {GPIO_PORT_D,GPIO_PIN_Px5},
    },
  },

  [HAL_TIMER_1] = {
    .tccra = &TCCR1A, .tccrb = &TCCR1B, .focr = &TCCR1C,
    .timsk = &TIMSK1, .tifr  = &TIFR1,
    .tcnt  = &TCNT1L, .ocra  = &OCR1AL, .ocrb = &OCR1BL,

    .caps     = TIMER_CAP_16BIT | TIMER_CAP_ICR | TIMER_CAP_EXT_CLK,
    .prr_mask = (1 << PRTIM1),
    .cs_mask  = (1 << CS12) | (1 << CS11) | (1 << CS10),
    .top_max  = UINT16_MAX,

    .pre_val   = pre_val_01,
    .pre_bits  = pre_bits_01,
    .pre_count = ARRAY_SIZE(pre_val_01),

    .timer_mode_table       = timer1,
    .timer_mode_table_count = ARRAY_SIZE(timer1),
    .oc_pin = {
      [TIMER_CH_A] = {GPIO_PORT_B,GPIO_PIN_Px1},
      [TIMER_CH_B] = {GPIO_PORT_B,GPIO_PIN_Px2},
    },
  },

  [HAL_TIMER_2] = {
    .tccra = &TCCR2A, .tccrb = &TCCR2B, .focr = &TCCR2B,
    .timsk = &TIMSK2, .tifr  = &TIFR2,
    .tcnt  = &TCNT2,  .ocra  = &OCR2A,  .ocrb = &OCR2B,

    .caps     = TIMER_CAP_ASYNC,
    .prr_mask = (1 << PRTIM2),
    .cs_mask  = (1 << CS22) | (1 << CS21) | (1 << CS20),
    .top_max  = UINT8_MAX,

    .pre_val   = pre_val_2,
    .pre_bits  = pre_bits_2,
    .pre_count = ARRAY_SIZE(pre_val_2),

    .timer_mode_table       = timer2,
    .timer_mode_table_count = ARRAY_SIZE(timer2),
    .oc_pin = {
      [TIMER_CH_A] = {GPIO_PORT_B,GPIO_PIN_Px3},
      [TIMER_CH_B] = {GPIO_PORT_D,GPIO_PIN_Px3},
    },
  },
};

static_assert(ARRAY_SIZE(timers) == HAL_TIMER_COUNT);

//-------------LOOKUP-TABLES-END--------------------

static status_t check_ready(timer_id_t t) {
  if (t >= HAL_TIMER_COUNT)  return STATUS_ERR_PARAM;
  if (!state[t].in_use)      return STATUS_ERR_STATE;
  if (state[t].suspended)    return STATUS_ERR_STATE;
  return STATUS_OK;
}

static void reg_write(const HAL_FLASH timer_desc_t *d,volatile uint8_t *reg, uint16_t v) {
  if (d->caps & TIMER_CAP_16BIT) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      *(volatile uint16_t *)reg = v;
    }
  } else {
    *reg = (uint8_t)v;
  }
}

static uint16_t reg_read(const HAL_FLASH timer_desc_t *d,volatile uint8_t *reg){
  if(d->caps & TIMER_CAP_16BIT){
    uint16_t v;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      v = *(volatile uint16_t *)reg;
    }
    return v;
  }
  return *reg;
}
status_t timer_init(timer_id_t t){
  
  if(HAL_TIMER_COUNT <= t) return STATUS_ERR_PARAM;
  if(state[t].in_use) return STATUS_ERR_BUSY;
  
  const HAL_FLASH timer_desc_t * d = &timers[t];

  power_enable(d->prr_mask);

  *d->tccrb = 0;               // Clock stop, WGMn2/WGM13 = 0
  *d->timsk = 0;               // No Interrupts
  *d->tccra = 0;               // COM-Bits, WGMn1:0
  *d->focr  = 0;
  reg_write(d, d->tcnt, 0);    
  reg_write(d, d->ocra, 0);
  reg_write(d, d->ocrb, 0);
  *d->tifr  = 0xFF;
  
  state[t] = (timer_state_t){ .in_use = true };

  return STATUS_OK;
}

static void write_wgm(const HAL_FLASH timer_desc_t * d,uint8_t wgm){
  *d->tccra = (uint8_t)(*d->tccra & (uint8_t)~0x03) | (wgm & 0x03);
  *d->tccrb = (uint8_t)(*d->tccrb & (uint8_t)~0x18) | ((wgm & 0x0C) << 1);
}

status_t timer_set_mode(timer_id_t t , timer_mode_t mode,timer_top_t top){
  status_t stat = check_ready(t);
  if(stat != STATUS_OK) return stat;
  if(TIMER_MODE_COUNT <= mode || TIMER_TOP_COUNT<=top) return STATUS_ERR_PARAM;
  uint8_t wgm =  0;
  const HAL_FLASH timer_desc_t *  d = &timers[t];
  for(; wgm<d->timer_mode_table_count;wgm++){
    if(d->timer_mode_table[wgm].mode == mode &&d->timer_mode_table[wgm].top_value == top){
      write_wgm(d,wgm);
      state[t].wgm=wgm;
      return STATUS_OK;
    }
  }

  return  STATUS_ERR_UNSUPPORTED;
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
static status_t timer_set_frequency(timer_id_t t,uint16_t ms, uint16_t * out_top){
  status_t stat = check_ready(t);
  if(stat != STATUS_OK) return stat;

  if(!out_top){return STATUS_ERR_PARAM;}

  const HAL_FLASH timer_desc_t *d = &timers[t];

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

  *d->tccrb = (uint8_t)(*d->tccrb & (uint8_t)~d->cs_mask) | d->pre_bits[i];
  *out_top = (uint16_t)(counts-1);

  return STATUS_OK;
}

status_t timer_start(timer_id_t t, uint16_t prescaler){
  status_t stat = check_ready(t);
  if(stat != STATUS_OK) return stat;

  const HAL_FLASH timer_desc_t * d = &timers[t];

  for(uint8_t i = 0 ; i < d->pre_count ; i++){
    if(d->pre_val[i] == prescaler){
      *d->tccrb = (uint8_t)((*d->tccrb & (uint8_t)~d->cs_mask) | d->pre_bits[i]);
      state[t].prev_presc = i;
      return STATUS_OK;
    }
  }

  return STATUS_ERR_PARAM;
}
status_t timer_stop(timer_id_t t){
  status_t stat = check_ready(t);
  if(stat != STATUS_OK) return stat;

  const HAL_FLASH timer_desc_t * d = &timers[t];
  *d->tccrb &= (uint8_t) ~d->cs_mask;

  return STATUS_OK;
}

ISR(TIMER1_COMPA_vect) {
  timer_callback_t f = cb;
  if (f != nullptr) f();
}