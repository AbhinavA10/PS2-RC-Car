/*==============================================================================
    Project: PS2-RC-Car          By: Abhi Agrahari
    Version: 1.2 				 Date: Jan 5th, 2018
    Target: RCCar1.0             Processor: PIC16F1459

    This is a project that controls an RC Car, from a wireless PS2 controller.

    The current step/project purpose is being able to send data back and forth
    on the PS2 controller
==============================================================================*/

#include	"xc.h"				// XC general include file

#include	"stdint.h"			// Include integer definitions
#include	"stdbool.h"			// Include Boolean (true/false) definitions

#include	"RCCar.h"			// Include RCCar constant symbols and functions

// TODO Set linker ROM ranges to 'default,-0-C03' under "Memory model" pull-down.
// TODO Set linker Code offset to '0xC04' under "Additional options" pull-down.
// TODO Set Instruction Freq. to 12MHz for simulator debugging

/*TODOLIST:
 * comments
 * check total time (time between polls should be 5-10ms?)
 * make array for databytes instead of separate variables
 * write code for pressure values, and motor mapping
 */

/*==============================================================================
 DECLARATIONS
==============================================================================*/
//-------- VARIABLES ---------------
unsigned char MOTORCMDVal = 0;
unsigned char DATAVal = 0;
unsigned char DATAByte2 = 0;
unsigned char DATAByte4 = 0;
unsigned char DATAByte5 = 0;
unsigned char DATAByte6 = 0;
unsigned char DATAByte7 = 0;
unsigned char DATAByte8 = 0;
unsigned char DATAByte9 = 0;
unsigned char servoPos = 90;
bool vibrateMotor = false;
bool analogMode = false;
bool analogPressureMode = false;

//-------- BUTTON BITS ---------------
#define BUTTON_SELECT       0b00000001
#define BUTTON_L3           0b00000010
#define BUTTON_R3           0b00000100
#define BUTTON_START        0b00001000
#define BUTTON_UP           0b00010000
#define BUTTON_RIGHT        0b00100000
#define BUTTON_DOWN         0b01000000
#define BUTTON_LEFT         0b10000000
//Above buttons are in data byte4, below are in data byte5
#define BUTTON_L2           0b00000001
#define BUTTON_R2           0b00000010
#define BUTTON_L1           0b00000100
#define BUTTON_R1           0b00001000
#define BUTTON_TRIANGLE     0b00010000
#define BUTTON_CIRCLE       0b00100000
#define BUTTON_X            0b01000000
#define BUTTON_SQUARE       0b10000000

/*==============================================================================
 TURN SERVO
    Function that takes a position for the servo as the input. It will send a
    pulse such that the timing, matches the servo position, through PWMing.
==============================================================================*/
void turnServo(unsigned char value) {
    SERVO = 1;
    __delay_us(540); //__delay_us(980);
    for (unsigned char i = 255 - value; i != 0; i--) {
        __delay_us(6); //__delay_us(4);
    }
    SERVO = 0;
}

/*==============================================================================
 SEND CMD RECEIVE DATA
    Function to simultaneously send a command byte when the clock goes low,
    and recieve the data byte, when the clock goes low. This is done bit by bit.
    The clock runs at ~250kHz.
    Input is the command to be sent, and Returns the value of the data received.
==============================================================================*/
unsigned char SendCMD_ReceiveDATA(unsigned char cmd) {
    unsigned char dat = 0;
    for (unsigned char i = 8; i != 0; i--) {
        CLOCKLINE = 0;
        if (cmd & 0b00000001 == 1) { //check the LSb of the command and compare it to "1"
            CMDLINE = 1; // send a 1 by setting CMDLine to high
        } else {
            CMDLINE = 0; //send a 0 by setting CMDLine Low
        }
        __delay_us(1);

        cmd = cmd >> 1; //shift command byte over, for the next bit when looped

        CLOCKLINE = 1;
        if (DATALINE == 1) {
            dat = (dat | 0b10000000); // add a value of 1 to the MSb of "data"
        }
        __delay_us(1);
        if (i != 1) dat = dat >> 1; // shift the contents over for the next bit if it's not the last bit
    }
    __delay_us(16);
    return dat;
}

/*==============================================================================
 DIGITAL POLL
    Function to
==============================================================================*/
void DigitalPoll() {
    ATTNLINE = 0;
    __delay_us(16);
    DATAVal = SendCMD_ReceiveDATA(0x01);
    DATAVal = SendCMD_ReceiveDATA(0x42);
    DATAVal = SendCMD_ReceiveDATA(0x00);
    DATAVal = SendCMD_ReceiveDATA(0x00);
    DATAVal = SendCMD_ReceiveDATA(0x00);
    ATTNLINE = 1;
    __delay_us(16);
}

/*==============================================================================
 ANALOG POLL
    Function to
==============================================================================*/
void AnalogPoll() {
    ATTNLINE = 0;
    __delay_us(16);
    DATAVal = SendCMD_ReceiveDATA(0x01);
    DATAVal = SendCMD_ReceiveDATA(0x42);
    DATAVal = SendCMD_ReceiveDATA(0x00);
    MOTORCMDVal = (vibrateMotor) ? 0xFF : 0x00;
    // above: if vibrate motor is set, command is set to switch on (0xFF)
    DATAByte4 = SendCMD_ReceiveDATA(MOTORCMDVal);
    DATAByte5 = SendCMD_ReceiveDATA(MOTORCMDVal);
    DATAByte6 = SendCMD_ReceiveDATA(0x00);
    DATAByte7 = SendCMD_ReceiveDATA(0x00);
    DATAByte8 = SendCMD_ReceiveDATA(0x00);
    DATAByte9 = SendCMD_ReceiveDATA(0x00);
    ATTNLINE = 1;
    __delay_us(16);
}

/*==============================================================================
 ENTER CONFIG
    Function to
==============================================================================*/
void EnterConfig() {
    ATTNLINE = 0;
    __delay_us(16);
    DATAVal = SendCMD_ReceiveDATA(0x01);
    DATAVal = SendCMD_ReceiveDATA(0x43);
    DATAVal = SendCMD_ReceiveDATA(0x00);
    DATAVal = SendCMD_ReceiveDATA(0x01); // shows entering config
    DATAVal = SendCMD_ReceiveDATA(0x00);
    ATTNLINE = 1;
    __delay_us(16);
}

/*==============================================================================
 TURN ON ANALOG MODE
    Function to
==============================================================================*/
void TurnOnAnalogMode() {
    ATTNLINE = 0;
    __delay_us(16);
    DATAVal = SendCMD_ReceiveDATA(0x01);
    DATAVal = SendCMD_ReceiveDATA(0x44);
    DATAVal = SendCMD_ReceiveDATA(0x00);
    DATAVal = SendCMD_ReceiveDATA(0x01);
    DATAVal = SendCMD_ReceiveDATA(0x03);
    DATAVal = SendCMD_ReceiveDATA(0x00);
    DATAVal = SendCMD_ReceiveDATA(0x00);
    DATAVal = SendCMD_ReceiveDATA(0x00);
    DATAVal = SendCMD_ReceiveDATA(0x00);
    ATTNLINE = 1;
    __delay_us(16);
}

/*==============================================================================
 MAP VIBRATION MOTORS
    Function to
==============================================================================*/
void MapVibrationMotors() {
    ATTNLINE = 0;
    __delay_us(16);
    DATAVal = SendCMD_ReceiveDATA(0x01);
    DATAVal = SendCMD_ReceiveDATA(0x4D);
    DATAVal = SendCMD_ReceiveDATA(0x00);
    DATAVal = SendCMD_ReceiveDATA(0x00); // maps this byte to control the small motor.
    DATAVal = SendCMD_ReceiveDATA(0x01); // maps this byte to control the large motor.
    DATAVal = SendCMD_ReceiveDATA(0xFF);
    DATAVal = SendCMD_ReceiveDATA(0xFF);
    DATAVal = SendCMD_ReceiveDATA(0xFF);
    DATAVal = SendCMD_ReceiveDATA(0xFF);
    ATTNLINE = 1;
    __delay_us(16);
}

/*==============================================================================
 TURN ON PRESSURE VALUE RETURN
    Function to
==============================================================================*/
void TurnonPressureValues() {
    ATTNLINE = 0;
    __delay_us(16);
    DATAVal = SendCMD_ReceiveDATA(0x01);
    DATAVal = SendCMD_ReceiveDATA(0x43);
    DATAVal = SendCMD_ReceiveDATA(0x00);
    DATAVal = SendCMD_ReceiveDATA(0x00); // shows exiting config
    DATAVal = SendCMD_ReceiveDATA(0x5A);
    DATAVal = SendCMD_ReceiveDATA(0x5A);
    DATAVal = SendCMD_ReceiveDATA(0x5A);
    DATAVal = SendCMD_ReceiveDATA(0x5A);
    DATAVal = SendCMD_ReceiveDATA(0x5A);
    ATTNLINE = 1;
    __delay_us(16);
}

/*==============================================================================
 EXIT CONFIG
    Function to
==============================================================================*/
void Exitconfig() {
    ATTNLINE = 0;
    __delay_us(16);
    DATAVal = SendCMD_ReceiveDATA(0x01);
    DATAVal = SendCMD_ReceiveDATA(0x43);
    DATAVal = SendCMD_ReceiveDATA(0x00);
    DATAVal = SendCMD_ReceiveDATA(0x00); // shows exiting config
    DATAVal = SendCMD_ReceiveDATA(0x5A);
    DATAVal = SendCMD_ReceiveDATA(0x5A);
    DATAVal = SendCMD_ReceiveDATA(0x5A);
    DATAVal = SendCMD_ReceiveDATA(0x5A);
    DATAVal = SendCMD_ReceiveDATA(0x5A);
    ATTNLINE = 1;
    __delay_us(16);
}

/*==============================================================================
 PS2 INIT
    Function to switch from digital to analog model
    Poll 3 times for initiation and refresh, enter config mode, switch and lock to
    analog, setup vibration motor to be toggled, and exit config mode
==============================================================================*/
void PS2_configToAnalog() {
    DigitalPoll();
    DigitalPoll();
    DigitalPoll();
    EnterConfig();
    TurnOnAnalogMode();
    MapVibrationMotors();
    if (analogPressureMode) {
        TurnonPressureValues();
    }
    Exitconfig();
    analogMode = true;
}

/*==============================================================================
 IS PRESSED
    Using masking, and bitwise operations, we can see if a button is pressed
    Since data line is normally high, and goes low when a button is pressed, we
    AND it and then check against 0.

    If the AND's result is zero, that means the button was pressed
    (since data line goes low when pressed). If the AND's result is not zero
    (bit was set), that means the button is not pressed
==============================================================================*/
bool isPressed(unsigned char dat, unsigned char key) {
    return ((dat & key) == 0);
}

/*==============================================================================
 MAIN
    The main() function is called first by the compiler.
==============================================================================*/
int main(void) {
    initOsc(); // Initialize oscillator
    initPorts(); // Initialize I/O pins and peripherals
    __delay_us(200);
    //PS2_configToAnalog(); //switch PS2 controller to analog mode
    while (1) {
        if (analogMode) {
            AnalogPoll();
            if (isPressed(DATAByte5, BUTTON_TRIANGLE)) {
                vibrateMotor = true;
            } else {
                vibrateMotor = false;
            }
            if (analogPressureMode) {
                // set motor command value to pressure value of button
            }
        } else {
            DigitalPoll();
        }
        if (isPressed(DATAByte5, BUTTON_TRIANGLE)) {
            LED2 = 1; // switch on headlights
            LED3 = 1;
        } else {
            LED2 = 0;
            LED3 = 0;
        }
        if (S1 == 0) // Enter the bootloader if S1 is pressed.
        {
            asm("movlp 0x00");
            asm("goto 0x001C");
        }
    }
}
