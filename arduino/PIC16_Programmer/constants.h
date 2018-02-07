/**
 * constants.h
 * 
 * Constants for Arduino code.
 * 
 * (C) Copyright 2018 Dennis J. McWherter, Jr.
 */

#ifndef CONSTANTS_H__
#define CONSTANTS_H__

#include <stdint.h>

/* Pin assignment for serial communication */
#define SERIAL_CLOCK_PORT 2
#define SERIAL_DATA_PORT 4

/* MCLR Pin designation for programming mode */
#define MCLR_CONTROL_PIN 13

/* Half of clock duty cycle in microseconds */
#define CLOCK_DELAY_MICROS 52

/* Delays */
/* Defined at: http://ww1.microchip.com/downloads/en/DeviceDoc/41208C.pdf */
#define TDLY2_MICROS 1
#define TERA_MICROS ((uint32_t) 10000)
#define TPROG_MICROS ((uint32_t) 2000)
#define TDIS_MICROS 100
#define TRESET_MICROS ((uint32_t) 10000)

#endif /* CONSTANTS_H__ */

