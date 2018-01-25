/*==============================================================================
    Project: PS2-RC-Car          By: Abhi Agrahari
    Version: 1.5 				 Date: Jan 24th, 2018
    Target: RCCar 1.0            Processor: PIC16F1459

    This is a project that controls an RC Car, from a wireless PS2 controller.

    The current project purpose is being able to send data back and forth
    on the PS2 controller.
==============================================================================*/

#include	"xc.h"				// XC general include file

#include	"stdint.h"			// Include integer definitions
#include	"stdbool.h"			// Include Boolean (true/false) definitions

#include	"RCCar.h"			// Include RCCar constant symbols and functions

// TODO Set linker ROM ranges to 'default,-0-C03' under "Memory model" pull-down.
// TODO Set linker Code offset to '0xC04' under "Additional options" pull-down.
// TODO Set Instruction Freq. to 12MHz for simulator debugging

/*TODOLIST:
 * use vibration motor (already configured)
 * use data nibble for controller type (created but unused)
 */

/*==============================================================================
 DECLARATIONS
==============================================================================*/
//-------- VARIABLES ---------------
unsigned char MOTORCMDVal = 0;
unsigned char arcDataValsByte[22]; // max is 21 bytes of data. I made 22 so we
//humans can easily read/reference the code, arcDataValsByte[0] is unused
unsigned char duration = 0;
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
//below are byte values for corresponding pressure buttons
#define P_BUTTON_RIGHT      10
#define P_BUTTON_LEFT       11
#define P_BUTTON_UP         12
#define P_BUTTON_DOWN       13
#define P_BUTTON_TRIANGLE   14
#define P_BUTTON_CIRCLE     15
#define P_BUTTON_X          16
#define P_BUTTON_SQUARE     17
#define P_BUTTON_L1         18
#define P_BUTTON_R1         19
#define P_BUTTON_L2         20
#define P_BUTTON_R2         21

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
 PWM LED 2
    Simple function to simply PWM an LED, based on input value. Used to
    demonstrate analog capabilities of PS2 Controller
==============================================================================*/
void pwmLED2(unsigned char brightness) {
    LED2 = 0;
    for (unsigned char counter = 255; counter != 0; counter--) {
        if (counter == brightness) LED2 = 1;
    }
}

/*==============================================================================
 SEND CMD RECEIVE DATA
    Function to simultaneously send a command byte when the clock goes low,
    and receive the data byte, when the clock goes high. This is done bit by bit.
    The clock runs at ~250kHz. -> about 4us per transfer
    Input is the command to be sent, and returns the value of the data received.
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
    __delay_us(16); // just a delay between bytes
    return dat;
}

/*==============================================================================
 SET ATTN LOW
    Function to set attention line to low. Created this function because it is
    used frequently. Signifies beginning of data transfer
==============================================================================*/
void setATTNLow() {
    ATTNLINE = 0;
    __delay_us(16);
}

/*==============================================================================
 SET ATTN HIGH
    Function to set attention line to high. Created this function because it is
    used frequently. Signifies end of data transfer.
==============================================================================*/
void setATTNHigh() {
    ATTNLINE = 1;
    __delay_us(16);
}

/*==============================================================================
 DIGITAL POLL
    Function to get digital button values from PS2 controller
    Set attention to low, then send and receive data.
==============================================================================*/
void DigitalPoll() {
    setATTNLow();
    arcDataValsByte[1] = SendCMD_ReceiveDATA(0x01);
    arcDataValsByte[2] = SendCMD_ReceiveDATA(0x42);
    arcDataValsByte[3] = SendCMD_ReceiveDATA(0x00);
    arcDataValsByte[4] = SendCMD_ReceiveDATA(0x00);
    arcDataValsByte[5] = SendCMD_ReceiveDATA(0x00);
    setATTNHigh();
}

/*==============================================================================
 ANALOG POLL
    Function to get digital button values, set vibration motor values, and get
    analog joystick values from PS2 controller
    Set attention to low, then send and receive data.
==============================================================================*/
void AnalogPoll() {
    setATTNLow();
    arcDataValsByte[1] = SendCMD_ReceiveDATA(0x01);
    arcDataValsByte[2] = SendCMD_ReceiveDATA(0x42);
    arcDataValsByte[3] = SendCMD_ReceiveDATA(0x00);
    arcDataValsByte[4] = SendCMD_ReceiveDATA(MOTORCMDVal);
    arcDataValsByte[5] = SendCMD_ReceiveDATA(MOTORCMDVal);
    arcDataValsByte[6] = SendCMD_ReceiveDATA(0x00);
    arcDataValsByte[7] = SendCMD_ReceiveDATA(0x00);
    arcDataValsByte[8] = SendCMD_ReceiveDATA(0x00);
    arcDataValsByte[9] = SendCMD_ReceiveDATA(0x00);
    setATTNHigh();
}

/*==============================================================================
 ENTER CONFIG
    Function to enter config mode in the PS2 Controller. Subsequent commands will
    actually configure the controller
==============================================================================*/
void EnterConfig() {
    setATTNLow();
    arcDataValsByte[1] = SendCMD_ReceiveDATA(0x01);
    arcDataValsByte[2] = SendCMD_ReceiveDATA(0x43);
    arcDataValsByte[3] = SendCMD_ReceiveDATA(0x00);
    arcDataValsByte[4] = SendCMD_ReceiveDATA(0x01); // shows entering config
    arcDataValsByte[5] = SendCMD_ReceiveDATA(0x00);
    setATTNHigh();
}

/*==============================================================================
 TURN ON ANALOG MODE
    Function to turn on the returning of analog joystick values, after entering
    config mode
==============================================================================*/
void TurnOnAnalogMode() {
    setATTNLow();
    arcDataValsByte[1] = SendCMD_ReceiveDATA(0x01);
    arcDataValsByte[2] = SendCMD_ReceiveDATA(0x44);
    arcDataValsByte[3] = SendCMD_ReceiveDATA(0x00);
    arcDataValsByte[4] = SendCMD_ReceiveDATA(0x01);
    arcDataValsByte[5] = SendCMD_ReceiveDATA(0x03);
    arcDataValsByte[6] = SendCMD_ReceiveDATA(0x00);
    arcDataValsByte[7] = SendCMD_ReceiveDATA(0x00);
    arcDataValsByte[8] = SendCMD_ReceiveDATA(0x00);
    arcDataValsByte[9] = SendCMD_ReceiveDATA(0x00);
    setATTNHigh();
}

/*==============================================================================
 MAP VIBRATION MOTORS
    Function to map the vibration motors to corresponding bytes4:5, after in
    config mode
==============================================================================*/
void MapVibrationMotors() {
    setATTNLow();
    arcDataValsByte[1] = SendCMD_ReceiveDATA(0x01);
    arcDataValsByte[2] = SendCMD_ReceiveDATA(0x4D);
    arcDataValsByte[3] = SendCMD_ReceiveDATA(0x00);
    arcDataValsByte[4] = SendCMD_ReceiveDATA(0x00); // maps this byte to control the small motor.
    arcDataValsByte[5] = SendCMD_ReceiveDATA(0x01); // maps this byte to control the large motor.
    arcDataValsByte[6] = SendCMD_ReceiveDATA(0xFF);
    arcDataValsByte[7] = SendCMD_ReceiveDATA(0xFF);
    arcDataValsByte[8] = SendCMD_ReceiveDATA(0xFF);
    arcDataValsByte[9] = SendCMD_ReceiveDATA(0xFF);
    setATTNHigh();
}

/*==============================================================================
 TURN ON PRESSURE VALUE RETURN
    Function to turn on the returning of pressure values from buttons. These
    values show how hard a digital buttton is being pressed. These information
    is returned in bytes 10:21 when switched on
    Excludes R3,L3, Select, and Start
==============================================================================*/
void TurnonPressureValues() {
    setATTNLow();
    arcDataValsByte[1] = SendCMD_ReceiveDATA(0x01);
    arcDataValsByte[2] = SendCMD_ReceiveDATA(0x4F);
    arcDataValsByte[3] = SendCMD_ReceiveDATA(0x00);
    arcDataValsByte[4] = SendCMD_ReceiveDATA(0xFF);
    arcDataValsByte[5] = SendCMD_ReceiveDATA(0xFF);
    arcDataValsByte[6] = SendCMD_ReceiveDATA(0x03);
    arcDataValsByte[7] = SendCMD_ReceiveDATA(0x00);
    arcDataValsByte[8] = SendCMD_ReceiveDATA(0x00);
    arcDataValsByte[9] = SendCMD_ReceiveDATA(0x00);
    setATTNHigh();
    analogPressureMode = true;
}

/*==============================================================================
 EXIT CONFIG
    Function to exit configuration mode
==============================================================================*/
void Exitconfig() {
    setATTNLow();
    arcDataValsByte[1] = SendCMD_ReceiveDATA(0x01);
    arcDataValsByte[2] = SendCMD_ReceiveDATA(0x43);
    arcDataValsByte[3] = SendCMD_ReceiveDATA(0x00);
    arcDataValsByte[4] = SendCMD_ReceiveDATA(0x00); // shows exiting config, as oppose to entering
    arcDataValsByte[5] = SendCMD_ReceiveDATA(0x5A);
    arcDataValsByte[6] = SendCMD_ReceiveDATA(0x5A);
    arcDataValsByte[7] = SendCMD_ReceiveDATA(0x5A);
    arcDataValsByte[8] = SendCMD_ReceiveDATA(0x5A);
    arcDataValsByte[9] = SendCMD_ReceiveDATA(0x5A);
    setATTNHigh();
}

/*==============================================================================
 PS2 INIT
    Function to switch from digital to analog model
    Poll 3 times for initiation and refresh, enter config mode, switch and lock to
    analog, setup vibration motor to be controlled, and possibly switch on the
    returning of pressure values if desired. Then, exit config mode
==============================================================================*/
void PS2_configToAnalog() {
    DigitalPoll();
    DigitalPoll();
    DigitalPoll();
    EnterConfig();
    TurnOnAnalogMode();
    MapVibrationMotors();
    //TurnonPressureValues();
    Exitconfig();
    analogMode = true; // set this boolean to true, so that different functions are called in main()
}

/*==============================================================================
 IS PRESSED
    Using masking, and bitwise operations, we can see if a button is pressed
    Since data line is normally high, and goes low when a button is pressed, we
    AND it and then check against 0.

    The button's value is something like `0b00001000`, with the `1` signifying
    which bit in the byte is its value.
    The Data byte will look like `0b11110111` if only the above button is pressed

    If the AND's result is zero, that means the button was pressed
    (since data line goes low when pressed). If the AND's result is not zero
    (bit was set), that means the button is not pressed
==============================================================================*/
bool isPressed(unsigned char dat, unsigned char key) {
    return ((dat & key) == 0);
}

/*==============================================================================
 GET LOWER NIBBLE
    Using bitwise operations, we can get the lower 4 bits of the data byte.
    The data's value is something like `0b01000001`. The lower nibble indicates 
    how many 16 bit words follow the header. ANDing the data with 0b00001111, we
    can get the lower 4 bits
==============================================================================*/
unsigned char getLowerNibble(unsigned char dat) {
    return (dat & 0b00001111);
}

/*==============================================================================
 GET HIGHER NIBBLE
    Using bitwise operations, we can get the upper 4 bits of the data byte.
    The data's value is something like `0b01000001`. The upper nibble indicates 
    indicates the mode of operation of the controller. Shifting data values to 
    the right 4 times, will give us the upper nibble
==============================================================================*/
unsigned char getUpperNibble(unsigned char dat) {
    return (dat >> 4);
}

/*==============================================================================
 BEEP
    Below functions are to create beeping noises. A separate function was needed
    for each note as __delay_us() cannot accept a variable as a parameter.
    The functions are used to display functioning of digital PS2 buttons.
==============================================================================*/
void beep(unsigned char period, unsigned char cycles) {
    for (cycles; cycles != 0; cycles--) {
        BEEPER = !BEEPER;
        for (period; period != 0; period--);
    }
}

void Eb4() {
    for (duration = 0; duration != 39; duration++) {
        BEEPER = 1;
        __delay_us(1607);
        BEEPER = 0;
        __delay_us(1607);
    }
}

void E4() {
    for (duration = 0; duration != 41; duration++) {
        BEEPER = 1;
        __delay_us(1517);
        BEEPER = 0;
        __delay_us(1517);
    }
}

void F4() {
    for (duration = 0; duration != 43; duration++) {
        BEEPER = 1;
        __delay_us(1432);
        BEEPER = 0;
        __delay_us(1432);
    }
}

void Fsharp() {
    for (duration = 0; duration != 46; duration++) {
        BEEPER = 1;
        __delay_us(1351);
        BEEPER = 0;
        __delay_us(1351);
    }
}

/*==============================================================================
 MAIN
    The main() function is called first by the compiler.
==============================================================================*/
int main(void) {
    initOsc(); // Initialize oscillator
    initPorts(); // Initialize I/O pins and peripherals
    __delay_us(200);
    PS2_configToAnalog(); //switch PS2 controller to analog mode
    //comment line out above to switch to digital only mode
    while (1) {
        if (analogMode) {
            AnalogPoll();
            if (isPressed(arcDataValsByte[5], BUTTON_SELECT)) {
                MOTORCMDVal = 0xFF; // switch on in next poll
            } else {
                MOTORCMDVal = 0x00; // switch off
            }
            if (analogPressureMode) {
                MOTORCMDVal = arcDataValsByte[P_BUTTON_R2];
                // Above line sets motor value to the returned button pressure
            }
            pwmLED2(arcDataValsByte[9]); // Y-axis on left joystick
            turnServo(arcDataValsByte[6]); //X-axis on right joystick
        } else {
            DigitalPoll();
        }
        if (isPressed(arcDataValsByte[5], BUTTON_TRIANGLE)) {
            Eb4();
        }
        if (isPressed(arcDataValsByte[5], BUTTON_SQUARE)) {
            E4();
        }
        if (isPressed(arcDataValsByte[5], BUTTON_CIRCLE)) {
            F4();
        }
        if (isPressed(arcDataValsByte[5], BUTTON_X)) {
            Fsharp();
        }
        if (S1 == 0) // Enter the bootloader
        {
            asm("movlp 0x00");
            asm("goto 0x001C");
        }
    }
}
