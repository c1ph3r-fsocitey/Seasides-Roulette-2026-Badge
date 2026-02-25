<img width="533" height="522" alt="image" src="https://github.com/user-attachments/assets/98cdd0f9-2353-4211-9b91-7898761de460" />

# Seasides 2026 Roulette Badge

## An ATtiny85-based interactive hardware CTF badge built for the Seasides 2026 Hardware Village.

### Over 300+ participants assembled this badge live during the event.

The badge features:
8-stage embedded CTF </br>
ATtiny85 firmware </br>
74HC595 shift register LED control </br>
UART-based challenge interface </br>
EEPROM-based progression </br>
Multiple animation modes </br>

## Hardware Overview
Microcontroller 
ATtiny85 (8 MHz internal oscillator) </br>
Shift Register
74HC595 (8-bit serial-in, parallel-out) </br>
LEDs
8 LEDs driven via 74HC595 </br>

## 🔌 Pinout
ATtiny85 → 74HC595 Mapping </br>
## ATtiny85 Pin	Function	Connected To </br>
PB3	SER (Data)	74HC595 SER (Pin 14) </br>
PB4	SRCLK	74HC595 SRCLK (Pin 11) </br>
PB0	RCLK	74HC595 RCLK (Pin 12) </br>
PB2	TX (Software UART TX)	UART Output </br>
PB1	RX / Button	UART RX + Button </br>
VCC	+5V / +3.3V	Power </br>
GND	Ground	Ground </br>
## 74HC595 Critical Pins </br>
Pin	Name	Connection </br> 
16	VCC	+5V / +3.3V </br>
8	GND	Ground </br>
13	OE	Must be tied to GND </br>
10	MR	Tied to VCC </br>
14	SER	From ATtiny85 PB3 </br>
11	SRCLK	From ATtiny85 PB4 </br>
12	RCLK	From ATtiny85 PB0 </br>

⚠️ Important: Pin 13 (OE) must be connected to GND. Floating CMOS control pins may cause unstable LED output.

## Flashing the Firmware
### Requirements
USBasp programmer </br>
AVR toolchain installed (avr-gcc, avrdude) </br>
ATtiny85 (internal 8 MHz clock) </br>

## 1️⃣ Compile Firmware
```
avr-gcc -mmcu=attiny85 -DF_CPU=8000000UL -Os -o badge.elf badge.c
avr-objcopy -O ihex -R .eeprom badge.elf badge.hex
avr-objcopy -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load \
  --change-section-lma .eeprom=0 badge.elf badge.eep
```
## 2️⃣ Flash Using USBasp
```
avrdude -c usbasp -p t85 -B 100 \
-U lfuse:w:0xE2:m \
-U flash:w:badge.hex \
-U eeprom:w:badge.eep
```

## Explanation:
lfuse 0xE2 → 8 MHz internal oscillator, no divide-by-8 </br>
-B 100 → Slower SCK for stable flashing

## 🔌 Connecting to Arduino as AVR ISP

### If using Arduino as programmer:
Upload ArduinoISP sketch to Arduino.
Connect: </br>
Arduino	ATtiny85 </br>
5V	VCC </br>
GND	GND </br>
D10	RESET </br>
D11	MOSI </br>
D12	MISO </br>
D13	SCK </br>

Then flash using:
```
avrdude -c arduino -P /dev/ttyUSB0 -b 19200 -p t85 -U flash:w:badge.hex
```

## UART Connection

The badge uses software UART at 1200 baud. </br>
Connect: </br>
PB2 → USB-TTL RX </br>
PB1 → USB-TTL TX </br>
GND → GND </br>

Then open terminal: </br>
screen /dev/ttyUSB0 1200 </br>
or </br>
picocom -b 1200 /dev/ttyUSB0 </br>

The badge will display the current challenge stage. </br>

## CTF Structure

8 stages </br>

Progressive unlocking via EEPROM </br>
Encoding challenges (ROT13, Base64, HEX, Binary, etc.) </br>
Each correct answer unlocks a new animation </br>
The goal is to understand, not brute-force. </br>

## Repository Contents

badge.c → Firmware </br>
/schematics → Circuit design </br>
/gerbers → PCB fabrication files </br>
/images → Assembled board photos </br>

## Lessons Learned
Never leave CMOS control pins floating </br>
Always bias OE and MR on shift registers </br>
Test hardware in low-noise environments </br>
Documentation matters as much as design </br>

## Author
Designed and built by Rahul Thareja (c1ph3r-fsocitey) </br>
Built for Seasides 2026 Hardware Village. </br>
