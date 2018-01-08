/*==============================================================================
    RC Car (PIC16F1459) hardware initialization and user functions.
==============================================================================*/

#include	"xc.h"				// XC compiler general include file

#include	"stdint.h"			// Integer definition
#include	"stdbool.h"			// Boolean (true/false) definition

#include	"RCCar.h"			// For optional user variable definitions

void initOsc(void) {
    OSCCON = 0xFC;              // 3x PLL enabled from 16MHz HFINTOSC
    ACTCON = 0x90;              // Enable active clock tuning from USB
    while (!PLLRDY);            // Wait for PLL lock
}

// Initialize hardware ports and peripherals. Set starting conditions.

void initPorts(void) {
    /*FROM PAGE 125 in datasheet
     *
     * TRISs are data directions  - 0 is output, 1 is input
     * PORTs are inputs
     * LATs are outputs
     * WPUs are weak pull-ups
     * ANSELs are analog selects - 0 is digital, 1 is analog
     */

    OPTION_REG = 0b01010111;    // Enable port pull-ups, TMR0 internal, div-256

    LATA = 0b00000000;          // Clear Port A latches before configuring PORTA
    ANSELA = 0b00000000;        // Make all Port A pins digital, only RA4 can be changed
    TRISA = 0b00001011;         //RA0, RA1 are used for USB, set Servo,ESC as output (bits4:5), S1 as input (bit3)
                                //bits 0:3, 6:7 cant be changed

    LATB = 0b00000000;          // Clear Port B latches before configuring PORTB
    ANSELB = 0b00000000;        // Make all Port B pins digital, only RB4:5 can be set
    TRISB = 0b10110000;         // bits 0:3 are default cleared, set clockline as out. dataline tiltswitches as inputs
    WPUB = 0b10110000;          //enable pins defined as inputs above, to all have internal pull-ups

    LATC = 0b00000000;          // Clear Port C latches before configuring PORTC
    ANSELC = 0b00000000;        // Make all Port C pins digital
    TRISC = 0b00000000;         // set all of PORT C as ouputs. RC0 is unused

    INTCON = 0;                 // Keep interrupts disabled for now
}
