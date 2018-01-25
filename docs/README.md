# PS2-RC-Car
This is a project that controls an RC Car, from a wireless PS2 controller.

The current project purpose is being able to send data back and forth on the PS2 controller.

- PIC Microcontroller is the master
- PS2 Controller is the slave

The MSSP/SPI Module in the PIC16F1459, transfers the MSB first (p197 of the datasheet). However, the PS2 protocol is to transfer LSB first. So, the SPI module was avoided (even though it was originally programmed to be used), and I have now  manually set pins, etc.

The circuit board I designed for this project can be found [here](https://circuits.io/circuits/5553911-rc-car)

## Notes on PS2 Interface

### Vocabulary
- Bit: one binary value
- Nibble: 4 bits
- Byte: 8 bits
- Packet: series of bytes
- CMD: Command/command line
- DATA: Data/Data line

### Pin Control
#### CLOCK
- Signal from PSX to Controller.
- Used to keep units in sync. Has to run at 250kHz
- cleared to low when sending Command, set high when receiving Data
- Since Command and Data are sent and received alternately within each bit, the clock is set to high and low alternately every bit.
- time interval time for each high and low combined is 4µs, and the interval of every byte sent is 16µs.

#### DATA (IN)
- Signal from Controller to PSX. (received)
- This signal is an 8 bit serial transmission synchronous to the falling edge of clock
- can ignore first three bytes, if we want to assume it's correct.

#### COMMAND (OUT)
- Signal from PSX to Controller. (Sent)
- 8 bit serial transmission on the falling edge of clock.

#### ATTENTION
- ATT is used to get the *attention* of the controller.
- This signal goes low for the duration of a transmission.
- i.e. a high again would represent the sending of packet has ended.

### Protocol  

Remote has 3 modes of operation
- Digital: 5Byte packets
- Analog: 9Byte packets
- Analog pressure: 21Byte packets

All transmissions are 8bit serial LSB first. All timing synchronous to the falling edge of the clock. (both incoming and outgoing signals change on a high to low transition of clock) Commands are sent on the falling edge, data is read on the rising edge. The play station sends a byte at the same time as it receives one.

When first plugged in,


| Byte#   | 1 | 2 | 3 | 4 | 5 |
| ------------- | ------------- | ------------- | ------------- |------------- |------------- |
| **COMMAND**  | `0x01` | ` 0x42`|`0x00` |`0x00` |`0x00` |
| **DATA**   |  `0xFF` | ` 0x41`|`0x5A` |`0xFF` |`0xFF` |

The first three bytes are known as a header. This is so the PS2 controller can send the data, after it receives the command.

BYTE 1: CMD's is always `0x01`. Data is always `0xFF`

BYTE 2:  
CMD:
Send [Main commands](https://github.com/AbhinavA10/PS2-RC-Car#main-commands-sent-by-cmd) to either poll controller or configure it.

DATA:
Device Mode - the high nibble `(0x4)` indicates the mode (`0x4` is digital, `0x7` is analog, `0xF` config), the lower nibble `(0x1)` is how many 16 bit words follow the header

BYTE 3: CMD's is always `0x00`. Data is always `0x5A`

BYTE 4:
CMD: to configure to control either vibration motor (`0xFF` signifies "on")
Data: each digital on/off button is mapped to one of the bits in 4th or 5th Bytes. `1` means unpressed

BYTE 5: same as byte 4

BYTE 6:
CMD: always `0x00`.
DATA: value of Analog Right X Axis `(Right:left, 0:255)`

BYTE 7:
CMD: always `0x00`.
DATA: value of Analog Right Y Axis (`Up:Down, 0:255)`

BYTE 8:
CMD: always `0x00`.
DATA: value of Analog Left X Axis `(Right:left, 0:255)`

BYTE 9:
CMD: always `0x00`.
DATA: value of Analog Left Y Axis `(Up:Down, 0:255)`

Ex. for 6th to 9th Byte: if data is `0x80 = 0b1000000 = 128`, the joystick is at the middle

BYTES 10-21 are button pressure values as described [here](https://github.com/AbhinavA10/PS2-RC-Car#analogpressure-button-mapping)

#### Digital Button Mapping:
| Button   | Select | L3(Stick) | R3(Stick) | Start | Up |Right |Down |Left |L2 |R2 |L1 |R1 |Triangle |Circle |X |Square |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
|**byte.bit**|	4.0	|4.1|	4.2|	4.3|	4.4	|4.5	|4.6	|4.7	|5.0|	5.1|	5.2|	5.3|	5.4|	5.5|	5.6|	5.7|


EX. If 4th Byte Data is `0x11 = 0b11110110`. The 0th and 4th bits are low. The rest is high. This means Select and Start are pressed simultaneously

#### Analog/Pressure Button Mapping:
| Button   | Right | Left| Up| Down| Triangle |Circle |X |Square |L1 |R1 |L2 |R2 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
|**Byte**|	10	|11|	12|	13|	14	|15	|16	|17	|18|	19|	20|	21|

#### Main Commands, sent by CMD
Data is not important in config mode and related CMDs. Focus is on the commands.

These main commands are sent in Byte2.

##### Polling: `0x42`
- 5byte packet for digital, 9byte packet for analog
- Motors can also be switched on after running polling command in bytes4/5 (once configured to be switchable in config mode)
- This is the most used command.

##### Enter Config Mode: `0x43`
- This packet is used to get in to configuration mode before setting up the controller to analogue mode or setup for the vibration motor.
- BYTE4 = `0x01`
- BYTE5 = `0x00`
- Config modes are always 9byte packets

##### Mode Selection: `0x44`
- This packet used to select the controller mode, after entering config mode
- BYTE4 = `0x01`: analog= 0x01, digital = 0x00
- BYTE5 = `0x03` to lock the controller into the mode sent in byte 4

##### Motor Mapping: `0x4D`
- This packet is used to setup the vibration motors function, after entering config mode
- `0x00` maps the corresponding byte in 0x42 to control the small motor.
- `0x01` maps the corresponding byte in 0x42 to control the large motor.
- `0xFF` disables, and is the default value when the controller is first connected
- BYTE4 = `0x00`
- BYTE5 = `0x01`

##### Pressure Values: `0x4F`
- This packet used to select the controller mode, after entering config mode
- BYTE4 = `0xFF`
- BYTE5 = `0xFF`
- BYTE6 = `0x03`

##### Exit Config Mode: `0x43`
- BYTE4 = `0x00`
- BYTES5->9 = `0x5A`
- Config modes are always 9byte packets

#### Recommended sequence to switch from digital to analog mode
1. Digital Poll (three times for initiation and refresh)
2. Enter configuration mode
3. Switch to analog mode and lock the analog mode
4. Setup vibration motor
5. Setup return of pressure values (optional)
6. Exit configuration mode
7. Analog Poll (looped forever)

## Sources

- [Gamesx PS2 Controller Protocol](https://www.gamesx.com/controldata/psxcont/psxcont.htm)
- [CuriousInventor's Protocol Explanation](http://htmlpreview.github.io/?https://github.com/AbhinavA10/PS2-RC-Car/blob/master/docs/Sources/Interfacing%20a%20PS2%20(PlayStation%202)%20Controller%20-%20CuriousInventor%20Tutorials.html) Note: this is not my website, I had saved the old version of their site, and am using it as such
- [Playstation Servo Controller Interface](docs/Sources/playstation-servo-controller-interface.pdf)
- [PIC16F1459 Datasheet](docs/Sources/PIC16F1459 Data Sheet.pdf)
- [Checking if a bit is set, in a byte](https://www.gamedev.net/forums/topic/657315-checking-if-a-bit-is-set-in-a-byte/)
- [Bit Field - Wikipedia](https://en.wikipedia.org/wiki/Bit_field)
