/*==============================================================================
	RC Car (PIC16F1459) symbolic constants for main and other user functions.
==============================================================================*/

// PORT I/O device definitions

#define Q1         	RA0			// Phototransistor light sensor input


// Clock frequency for delay macros and simulation

#define _XTAL_FREQ	32000000	// Set clock frequency for time delays
#define FCY	_XTAL_FREQ/4        // Set processor instruction cycle time

// TODO - Add function prototypes for all functions in RAINBO.c here:

void init(void);                // Initialization function prototype