/*==============================================================================
	RC Car (PIC16F1459) hardware initialization and user functions.
==============================================================================*/

#include	"xc.h"				// XC compiler general include file

#include	"stdint.h"			// Integer definition
#include	"stdbool.h"			// Boolean (true/false) definition

#include	"RCCar.h"			// For optional user variable definitions

void init(void)
{
	// Initialize oscillator
	
	OSCCON = 0b11110000;		// 4xPLL with 8 MHz internal oscillator = 32 MHz
	//while(!PLLR);				// Wait until PLL is locked for timing
	
	
	
    INTCON = 0;                 // Keep interrupts disabled for now
}
