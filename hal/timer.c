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
  volatile uint8_t           *focr; // FOCnA (Bit 7) / FOCnB (Bit 6): T0/T2 TCCRnB, T1 TCCR1C
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

  // WGM table , index = WGM-value
  const HAL_FLASH wgm_mode_t *timer_mode_table;
  uint8_t                     timer_mode_table_count;

  gpio_id_t oc_pin[2];
} timer_desc_t;

typedef struct {
  bool    in_use;
  bool    suspended;    // PRR-Bit set, Register not accessible
  uint8_t wgm;          // current wgm value
  uint8_t prev_presc;   // last selected prescaler
  uint16_t top;         // last TOP written by timer_set_frequency
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

status_t timer_deinit(timer_id_t t){
  if(t >= HAL_TIMER_COUNT) return STATUS_ERR_PARAM;
  if(!state[t].in_use) return STATUS_ERR_STATE;

  const HAL_FLASH timer_desc_t *d = &timers[t];

  power_enable(d->prr_mask); // if supended, so registers can be written

  *d->tccrb = 0;               // Clock stop, WGMn2/WGM13 = 0
  *d->timsk = 0;               // No Interrupts
  *d->tccra = 0;               // COM-Bits, WGMn1:0
  *d->focr  = 0;
  *d->tifr  = 0xFF;

  if(t == HAL_TIMER_1) cb = nullptr; // TODO: per-timer/per-IRQ callbacks

  power_disable(d->prr_mask);

  state[t] = (timer_state_t){0};

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
 * @brief  Writes TOP into the register the active WGM mode uses as TOP source
 *         (OCRnA or ICR1). Does not touch the CS bits.
 *
 * @retval STATUS_ERR_PARAM  top == 0 or top > d->top_max
 * @retval STATUS_ERR_STATE  Mode has a fixed TOP (Normal, 0x00FF, ...)
 */
static status_t timer_set_top(timer_id_t t, uint16_t top){
  const HAL_FLASH timer_desc_t *d = &timers[t];
  if(top == 0 || top > d->top_max) return STATUS_ERR_PARAM;

  switch(d->timer_mode_table[state[t].wgm].top_value){
    case TIMER_TOP_OCRA:
      reg_write(d, d->ocra, top);
      return STATUS_OK;
    case TIMER_TOP_ICR1:
      if(!(d->caps & TIMER_CAP_ICR)) return STATUS_ERR_UNSUPPORTED;
      reg_write(d, (volatile uint8_t *)&ICR1, top);
      return STATUS_OK;
    default:
      return STATUS_ERR_STATE;
  }
}

/** Single vs. dual slope for the WGM mode; STATUS_ERR_STATE for Normal (fixed TOP). */
static status_t mode_slope(const HAL_FLASH timer_desc_t *d, uint8_t wgm, bool *dual_slope){
  const timer_mode_t mode = d->timer_mode_table[wgm].mode;

  if (mode == TIMER_MODE_CTC || mode == TIMER_MODE_FAST_PWM) {
    *dual_slope = false;
  } else if (mode == TIMER_MODE_PHASE_CORRECT_PWM ||
             mode == TIMER_MODE_PHASE_FREQ_CORRECT_PWM) {
    *dual_slope = true;
  } else {
    return STATUS_ERR_STATE;
  }
  return STATUS_OK;
}

/**
 * @brief  Picks the smallest prescaler N for which the requested period fits
 *         into the timer, and computes the matching TOP value.
 *
 * Pure calculation: no registers are written, the timer is not started.
 * Callers (timer_set_frequency, timer_set_period_ms) convert their input to
 * CPU ticks first, write TOP via timer_set_top and only then the CS bits.
 *
 * Formulas (ATmega328P datasheet, timer chapters, mode descriptions):
 *
 *   CTC / Fast PWM        f = F_CPU / (N * (TOP + 1))     counts 0..TOP
 *   Phase (+Freq) Correct f = F_CPU / (2 * N * TOP)       counts 0..TOP..0
 *
 * With cpu_ticks = F_CPU / f (CPU clocks per period):
 *
 *   div    = N          (CTC / Fast PWM)
 *          = 2 * N      (Phase Correct / Phase+Freq Correct)
 *   counts = round(cpu_ticks / div) = (cpu_ticks + div/2) / div
 *   TOP    = counts - 1 (CTC / Fast PWM)
 *          = counts     (Phase Correct / Phase+Freq Correct)
 *
 * A prescaler fits if 1 <= TOP <= d->top_max (counts >= 2).
 * The smallest fitting N is chosen because it gives the largest TOP and
 * therefore the finest frequency / duty-cycle resolution.
 *
 * Examples (F_CPU = 16 MHz):
 *   Timer0, CTC,           1 kHz -> cpu_ticks 16000,  N 64, TOP 249
 *   Timer1, CTC,           1 kHz -> cpu_ticks 16000,  N 1,  TOP 15999
 *   Timer1, Phase Correct, 50 Hz -> cpu_ticks 320000, N 8,  TOP 20000
 *
 * Overflow: div <= 2 * 1024, cpu_ticks + div/2 stays far below 2^32
 * for every cpu_ticks a uint32_t caller can produce from F_CPU.
 *
 * @param d          Timer descriptor (flash)
 * @param wgm        Active WGM index (state[t].wgm), selects the mode
 * @param cpu_ticks  Requested period in CPU clock cycles (F_CPU / f)
 * @param pre_idx    [out] Index into d->pre_val / d->pre_bits
 * @param top        [out] TOP value for OCRnA or ICR1
 *
 * @retval STATUS_OK         pre_idx and top are valid
 * @retval STATUS_ERR_PARAM  NULL pointer, or no prescaler fits
 *                           (frequency too high or too low for this timer)
 * @retval STATUS_ERR_STATE  Mode has a fixed TOP (Normal mode); only the
 *                           prescaler can be chosen there
 *
 * @note Outputs are only written on STATUS_OK.
 */
static status_t calc_pre_top(const HAL_FLASH timer_desc_t *d, uint8_t wgm,
                             uint32_t cpu_ticks, uint8_t *pre_idx, uint16_t *top){

  if (!pre_idx || !top) return STATUS_ERR_PARAM;

  bool dual_slope;
  status_t s = mode_slope(d, wgm, &dual_slope);
  if (s != STATUS_OK) return s;

  for(uint8_t i = 0; i < d->pre_count;i++){
    uint32_t div = (uint32_t) d->pre_val[i] * (dual_slope ? 2u : 1u);

    uint32_t counts = (cpu_ticks + div / 2) / div;

    // CTC/Fast PWM: top = counts - 1,  Phase Correct: top = counts
    uint32_t t;
    if (counts >= 2){
      t = dual_slope ? counts:counts-1;
    }else{
      t = counts;
    }

    if(counts >= 2 && t <= d->top_max){
      *pre_idx = i;
      *top = (uint16_t) t;
      return STATUS_OK;
    }
  }

  return STATUS_ERR_PARAM;
}

status_t timer_set_frequency(timer_id_t t, uint32_t hz, uint32_t *actual_hz){
  status_t s = check_ready(t);
  if (s != STATUS_OK) return s;
  if (hz == 0 || !actual_hz) return STATUS_ERR_PARAM;

  const HAL_FLASH timer_desc_t *d = &timers[t];
  const uint8_t wgm = state[t].wgm;

  uint8_t  pre_idx;
  uint16_t top;
  s = calc_pre_top(d, wgm, F_CPU / hz, &pre_idx, &top);
  if (s != STATUS_OK) return s;

  bool dual_slope;
  s = mode_slope(d, wgm, &dual_slope);
  if (s != STATUS_OK) return s;

  s = timer_set_top(t, top);
  if (s != STATUS_OK) return s;

  state[t].top        = top;
  state[t].prev_presc = pre_idx; 

  const uint32_t n = d->pre_val[pre_idx];
  *actual_hz = dual_slope ? F_CPU / (2u * n * top)
                          : F_CPU / (n * ((uint32_t)top + 1u));
  return STATUS_OK;
}

status_t timer_start(timer_id_t t, uint16_t prescaler){
  status_t stat = check_ready(t);
  if(stat != STATUS_OK) return stat;

  const HAL_FLASH timer_desc_t * d = &timers[t];

  *d->tccrb = (uint8_t)((*d->tccrb & (uint8_t)~d->cs_mask) | d->pre_bits[state[t].prev_presc]);

  return STATUS_OK;
}
status_t timer_stop(timer_id_t t){
  status_t stat = check_ready(t);
  if(stat != STATUS_OK) return stat;

  const HAL_FLASH timer_desc_t * d = &timers[t];
  *d->tccrb &= (uint8_t) ~d->cs_mask;

  return STATUS_OK;
}

status_t timer_suspend(timer_id_t t){
  if(t>=HAL_TIMER_COUNT) return STATUS_ERR_PARAM;
  if(!state[t].in_use) return STATUS_ERR_STATE;
  if (state[t].suspended)   return STATUS_OK;

  const HAL_FLASH timer_desc_t * d = &timers[t];

  if((d->caps & TIMER_CAP_ASYNC) && ( ASSR & _BV(AS2))) return STATUS_ERR_STATE;
  
  power_disable(d->prr_mask);
  state[t].suspended = true;
  return STATUS_OK;
}

status_t timer_resume(timer_id_t t){
  if(t>=HAL_TIMER_COUNT) return STATUS_ERR_PARAM;
  if(!state[t].in_use) return STATUS_ERR_STATE;
  if (!state[t].suspended) return STATUS_OK;

  const HAL_FLASH timer_desc_t * d = &timers[t];
  // Timer state Frozen so no additional regs need to be written
  power_enable(d->prr_mask);

  state[t].suspended = false;

  return STATUS_OK;
}

ISR(TIMER1_COMPA_vect) {
  timer_callback_t f = cb;
  if (f != nullptr) f();
}