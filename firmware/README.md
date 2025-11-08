# HEMA Jig Firmware

Firmware for the PIC18F4550 microcontroller that controls the automatic cartridge scanning jig. This firmware communicates with a Raspberry Pi via UART to coordinate QR code scanning and validation of medical device cartridges.

## Overview

The jig firmware manages:
- **Pneumatic cylinder control** for cartridge advancement
- **UART communication** with Raspberry Pi at 115200 baud
- **GPIO handshaking** for synchronization
- **Sensor monitoring** (safety, position, stack presence)
- **LED indicators and buzzer** for operator feedback
- **Button interface** for manual control

## Hardware Requirements

### Microcontroller
- **PIC18F4550** running at 48MHz
- 20MHz crystal with PLL enabled
- USB voltage regulator (if USB is used)

### Connections

#### UART (Raspberry Pi Communication)
- **RC6** (Pin 23) - UART TX → Raspberry Pi RX
- **RC7** (Pin 24) - UART RX ← Raspberry Pi TX
- **GND** - Common ground between PIC and Pi

#### GPIO Handshake Lines
- **RB6** (Pin 36) - RASP_IN_PIC ← Pi GPIO 18 (Pi status: HIGH=ready, LOW=busy)
- **RB5** (Pin 35) - INT_PIC → Pi GPIO 17/24 (Interrupt to Pi)
- **RB7** (Pin 37) - SHD_PIC → Pi GPIO 27/25 (Shutdown signal)

#### Pusher Mechanism (Pneumatic Cylinder)
- **RA0** (Pin 2) - PUSHER_EXTEND (solenoid valve)
- **RA1** (Pin 3) - PUSHER_RETRACT (solenoid valve)

#### Stopper Mechanism
- **RA2** (Pin 4) - STOPPER_UP
- **RA3** (Pin 5) - STOPPER_DOWN

#### Sensors (Active Low with Pull-ups)
- **RB0** (Pin 33) - SENSOR_STACK_PRESENT
- **RB1** (Pin 34) - SENSOR_AT_SCANNER
- **RB2** (Pin 35) - SENSOR_PUSHER_EXTENDED
- **RB3** (Pin 36) - SENSOR_PUSHER_RETRACTED
- **RB4** (Pin 37) - SENSOR_SAFETY_OK

#### Indicators
- **RC0** (Pin 15) - LED_GREEN (Pass)
- **RC1** (Pin 16) - LED_RED (Reject)
- **RC2** (Pin 17) - LED_YELLOW (Duplicate/Warning)
- **RD0** (Pin 19) - BUZZER

#### User Interface
- **RE0** (Pin 8) - BTN_START (active low)
- **RE1** (Pin 9) - BTN_STOP (active low)
- **RE2** (Pin 10) - BTN_RESET (active low)

## Communication Protocol

### Command Flow

```
PIC → Pi: Send scan request (0x14 or 0x13)
Pi: Set RASP_IN_PIC LOW (busy)
Pi: Process QR scan and validation
Pi → PIC: Send response ('A', 'R', 'D', etc.)
Pi: Set RASP_IN_PIC HIGH (ready)
PIC: Process response and advance cartridge
```

### Commands (PIC → Pi)
- `0x14` (20) - Scan request with retry capability
- `0x13` (19) - Final scan attempt (no more retries)
- `0x00` (0) - Stop/reset command

### Responses (Pi → PIC)
- `A` - **Accept/Pass** - QR valid, advance to good bin
- `R` - **Reject/Fail** - QR invalid, advance to reject bin
- `D` - **Duplicate** - QR already scanned, reject
- `S` - **Scanner Error** - Hardware/software error
- `Q` - **No QR** - No QR code detected, retry
- `L` - **Length Error** - QR wrong length, reject
- `B` - **Logging Error** - Database/file error
- `C` - **Repeat Test** - Cartridge already tested
- `H` - **Hardware Error** - Pi hardware failure

### Timing
- **UART Baud Rate**: 115200 bps
- **Response Timeout**: 12 seconds
- **Pusher Extend**: 400ms
- **Pusher Retract**: 400ms
- **Settle Time**: 200ms
- **Detection Timeout**: 3 seconds
- **Scan Timeout**: 5 seconds
- **Max Retries**: 3 attempts

## Building the Firmware

### Prerequisites

1. **MPLAB XC8 Compiler** (free version is sufficient)
   - Download from: https://www.microchip.com/mplab/compilers
   - Install and add to system PATH

2. **Make** (optional, for using Makefile)
   - Linux/Mac: Usually pre-installed
   - Windows: Install from GnuWin32 or use MSYS2

### Build Steps

#### Using Makefile (Recommended)

```bash
# Build firmware
make all

# Clean build artifacts
make clean

# Show help
make help
```

#### Using MPLAB X IDE

1. Create a new project for PIC18F4550
2. Add all `.c` and `.h` files to the project
3. Configure compiler: XC8, optimization level default
4. Build project (Production → Build Main Project)

#### Using Command Line

```bash
# Compile source files
xc8 --chip=18F4550 --pass1 main.c
xc8 --chip=18F4550 --pass1 uart.c
xc8 --chip=18F4550 --pass1 jig_control.c

# Link
xc8 --chip=18F4550 -o hema_jig_firmware main.p1 uart.p1 jig_control.p1
```

## Programming the Device

### Using PICkit3/PICkit4

```bash
# Using pk2cmd
pk2cmd -P PIC18F4550 -F hema_jig_firmware.hex -M -R

# Using MPLAB IPE (GUI)
# 1. Launch MPLAB IPE
# 2. Select device: PIC18F4550
# 3. Connect programmer
# 4. Load hex file
# 5. Program device
```

### Using MPLAB X IDE

1. Connect PICkit programmer to PC and PIC
2. In MPLAB X: Run → Program Device

## Configuration

### Timing Adjustments

Edit `config.h` to adjust timing parameters:

```c
#define PUSH_EXTEND_MS 400      // Pusher extension time
#define PUSH_RETRACT_MS 400     // Pusher retraction time
#define SETTLE_MS 200           // Settle time after movement
#define DETECT_TIMEOUT_MS 3000  // Cartridge detection timeout
#define SCAN_TIMEOUT_MS 5000    // QR scan timeout
#define MAX_SCAN_RETRIES 3      // Maximum retry attempts
```

### Pin Remapping

Edit `pin_definitions.h` to change I/O pin assignments if your hardware differs.

## Operation

### Startup Sequence

1. Power on system
2. PIC initializes (LEDs flash, beep)
3. PIC waits for Raspberry Pi ready signal (RASP_IN_PIC HIGH)
4. Green LED blinks briefly when Pi is ready
5. System enters idle state

### Running Operation

1. Load cartridges into stack
2. Press **START** button
3. System automatically:
   - Checks safety interlock
   - Advances cartridge to scanner position
   - Requests QR scan from Pi
   - Waits for validation response
   - Routes cartridge based on result (pass/reject)
   - Repeats for next cartridge
4. Press **STOP** button to pause
5. Press **RESET** for emergency stop

### LED Indicators

- **Green**: Pass (good cartridge)
- **Red**: Reject or error
- **Yellow**: Scanning in progress / waiting
- **Flashing Red**: Emergency stop / critical error

### Buzzer Patterns

- **Short beep (100ms)**: Successful scan (pass)
- **Long beep (200ms)**: Reject
- **Two beeps**: Duplicate QR code
- **Triple beep**: Error condition

## Troubleshooting

### No Communication with Pi

1. Check UART connections (TX ↔ RX, GND common)
2. Verify baud rate match (115200)
3. Check RASP_IN_PIC signal level (should be HIGH when Pi ready)
4. Use logic analyzer to verify signal timing

### Pusher Not Moving

1. Check solenoid valve connections (RA0, RA1)
2. Verify pneumatic air supply
3. Check sensor feedback (RB2, RB3)
4. Ensure safety interlock is engaged (RB4 LOW)

### No Cartridge Detection

1. Check sensor at scanner position (RB1)
2. Verify sensor wiring and power
3. Adjust sensor position/sensitivity
4. Check for mechanical obstructions

### Pi Not Responding

1. Verify Pi is powered and booted
2. Check Pi software is running (`main.py`)
3. Verify GPIO 18 configuration
4. Check serial port settings in Pi

### Random Resets

1. Check power supply stability (5V regulated)
2. Add decoupling capacitors (100nF near VDD pins)
3. Enable brown-out reset protection
4. Check for EMI from solenoids (add flyback diodes)

## Safety Features

- **Safety Interlock**: Must be engaged (RB4 LOW) for operation
- **Emergency Stop**: RESET button immediately de-energizes all actuators
- **Watchdog Timer**: Can be enabled for automatic recovery
- **Brown-out Reset**: Protects against power supply fluctuations

## File Structure

```
firmware/
├── main.c              # Main program and state machine
├── uart.c              # UART communication implementation
├── uart.h              # UART function declarations
├── jig_control.c       # Jig mechanism control
├── jig_control.h       # Jig control interface
├── config.h            # System configuration constants
├── pin_definitions.h   # I/O pin assignments and macros
├── Makefile            # Build system
└── README.md           # This file
```

## Integration with Raspberry Pi

This firmware is designed to work with the Python application running on Raspberry Pi. See `python/DEPLOYMENT_GUIDE.md` for:

- Pi-side software setup
- GPIO configuration
- UART enabling
- Protocol details
- Full system integration

## Version History

### v1.0.0 (Initial Release)
- UART communication at 115200 baud
- Basic jig control (pusher extend/retract)
- GPIO handshaking with Pi
- Sensor monitoring
- LED and buzzer feedback
- Button interface
- Retry logic for failed scans
- Safety interlock support

## License

Copyright (c) 2024 MOLBIO Diagnostics Limited
Developed by QA Team Site-III

## Support

For technical support:
- Check hardware connections match pin definitions
- Verify configuration matches your setup
- Use logic analyzer for protocol debugging
- Review UART communication logs
- Contact development team for firmware updates

---

**IMPORTANT**: This is production hardware controlling mechanical actuators. Always ensure:
- Safety interlocks are properly installed
- Emergency stop is accessible
- Personnel are trained on operation
- Regular maintenance is performed
- Firmware updates are tested in development environment first
