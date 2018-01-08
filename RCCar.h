/*==============================================================================
    RC Car (PIC16F1459) symbolic constants for main and other user functions.
==============================================================================*/

/* Inputs read the Port registers (eg. RC0). Outputs write to the port latches
(eg. LATC0).
 */

// PORT I/O device definitions

#define SERVO           LATA5             // Servo Output
#define ESC             LATA4             // Electronic Speed Controller Output
#define S1              RA3               // Button (ex. reset)
#define BEEPER          LATC5             // Piezo Element Output
#define VIBRATIONMOTOR  LATC3             // Pin 3 of PS2 Remote - Vibration Motor
#define ATTNLINE        LATC6             // Pin 6 of PS2 Remote - Attention Line (SS)
#define CMDLINE         LATC7             // Pin 2 of PS2 Remote - Command Line (SDO)
#define TILTSWITCH2     RB7               // Tilt Switch #2 (Pointing Up)

#define LED2            LATC1             // Output for LED2 - Headlight LED
#define LED3            LATC2             // Output for LED3 - Headlight LED
#define DATALINE        RB4               // Pin 1 of PS2 Remote - Data Line (SDI)
#define TILTSWITCH1     RB5               // Tilt Switch #1 (Pointing Down)
#define CLOCKLINE       LATB6             // Pin 7 of PS2 Remote - Clock Line (SCK)

// Clock frequency for delay macros and simulation

#define _XTAL_FREQ	48000000	// Set clock frequency for time delays

// TODO - Add function prototypes for all functions in RCCar.c here:

void initOsc(void); // Oscillator initialization function prototype.
void initPorts(void); // Port initialization function prototype.
