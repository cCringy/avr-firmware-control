// hal/power_priv.h – nur für HAL-Treiber, nie aus app/ inkludieren
#ifndef POWER_INTERN_H
#define POWER_INTERN_H

#include <avr/io.h>
#include <util/atomic.h>

static inline void power_enable(uint8_t prr_mask) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { PRR &= (uint8_t)~prr_mask; }
}

static inline void power_disable(uint8_t prr_mask) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { PRR |= prr_mask; }
}

#endif