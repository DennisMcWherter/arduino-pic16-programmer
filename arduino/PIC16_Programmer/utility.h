/**
 * utility.h
 * 
 * A set of inline helper methods for signaling Arduino pins.
 *
 * (C) Copyright 2018 Dennis J. McWherter, Jr. All rights reserved.
 */

#ifndef UTILITY_H__
#define UTILITY_H__

#include "Arduino.h"
#include "constants.h"

inline bool is_clock_low() {
  return !(PORTD & _BV(SERIAL_CLOCK_PORT));
}

inline bool clock_tick() {
  if (!is_clock_low()) {
    PORTD &= ~_BV(SERIAL_CLOCK_PORT);
    return false;
  }
  PORTD |= _BV(SERIAL_CLOCK_PORT);
  return true;
}

inline void clock_low() {
  PORTD &= _BV(SERIAL_CLOCK_PORT);
}

inline void data_low() {
  PORTD &= ~_BV(SERIAL_DATA_PORT);
}

inline void data_high() {
  PORTD |= _BV(SERIAL_DATA_PORT);
}

inline void data_write_mode() {
    DDRD |= _BV(SERIAL_DATA_PORT);
}

inline void data_read_mode() {
    DDRD &= ~_BV(SERIAL_DATA_PORT);
}

inline bool read_data_bit() {
    return PIND & SERIAL_DATA_PORT;
}

inline void mclr(unsigned sig) {
  if (sig == HIGH) {
    analogWrite(MCLR_CONTROL_PIN, 0);
  } else {
    digitalWrite(MCLR_CONTROL_PIN, HIGH);
  }
}

#endif /* UTILITY_H__ */
