/*
 * File:   main.c
 * Author: dennis
 */

#pragma config OSC = RC         // Oscillator selection bits (RC oscillator)
#pragma config WDT = OFF         // Watchdog timer enable bit (WDT enabled)
#pragma config CP = OFF         // Code protection bit (Code protection off

#include <xc.h>
#include <pic16f57.h>

void main(void) {
    TRISB = 0; // Everything as outputs!
    // Turn all bits high.
    PORTB = 0xff;
    while (1) {
        RB5 = 1;
        RB4 = 0;
    }
}

