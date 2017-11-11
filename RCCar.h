/*==============================================================================
    RC Car (PIC16F1459) symbolic constants for main and other user functions.
==============================================================================*/
// need to check how to reference SPI
// need to check reset button
// check clock freq.

// Inputs read the Port registers (eg. RC0). Outputs write to the port latches
// (eg. LATC0).

// PORT I/O device definitions

#define SERVO       LATA5             // Servo Output
#define ESC         LATA4             // Electronic Speed Controller Output
#define S1          LATA3             //Button (ex. reset)
#define MICROPHONE  RC5             // Piezo Element Input            
#define BEEPER      LATC5             // Piezo Element Output            
#define PS2PIN3     LATC3             // Pin 3 of PS2 Remote - Vibration Motor
#define PS2PIN6     LATC6             // Pin 6 of PS2 Remote - Attention Line (SS)
#define PS2PIN2     LATC7             // Pin 2 of PS2 Remote - Command Line (SDO)
#define TILTSWITCH2 RB7             // Tilt Switch #2 (Pointing Up)

#define DPLUS       RA0
#define DMINUS      RA1
#define LED2        LATC1             // Output for LED2 - Headlight LED
#define LED3        LATC2             // Output for LED3 - Headlight LED
#define PS2PIN1     RB4             // Pin 1 of PS2 Remote - Data Line (SDI)
#define TILTSWITCH1 RB5             // Tilt Switch #1 (Pointing Down)
#define PS2PIN7     LATB6             // Pin 7 of PS2 Remote - Clock Line (SCK)

// Clock frequency for delay macros and simulation

#define _XTAL_FREQ	32000000	// Set clock frequency for time delays
#define FCY	_XTAL_FREQ/4        // Set processor instruction cycle time

// TODO - Add function prototypes for all functions in RAINBO.c here:

void init(void); // Initialization function prototype