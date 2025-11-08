# HEMA Automatic Cartridge Scanning Jig

Complete hardware and software system for automated QR code scanning and validation of medical device cartridges.

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    HEMA Jig System                           │
└─────────────────────────────────────────────────────────────┘

┌──────────────────┐         ┌──────────────────┐
│  PIC18F4550      │ UART    │  Raspberry Pi    │
│  Firmware        │◄───────►│  Python App      │
│  (C Code)        │ 115200  │  (QR Validation) │
└──────────────────┘         └──────────────────┘
        │                            │
        │ GPIO Handshake             │
        ├────────────────────────────┤
        │                            │
┌───────┴────────┐          ┌────────┴─────────┐
│  Jig Hardware  │          │   QR Scanner     │
│  - Pusher      │          │   LCD Display    │
│  - Sensors     │          │   User Interface │
│  - LEDs        │          │                  │
└────────────────┘          └──────────────────┘
```

## Repository Structure

```
HEMA/
├── firmware/                    # PIC18F4550 microcontroller firmware
│   ├── main.c                  # Main program and state machine
│   ├── uart.c / uart.h         # UART communication (115200 baud)
│   ├── jig_control.c / .h      # Jig mechanism control
│   ├── pin_definitions.h       # I/O pin assignments
│   ├── config.h                # System configuration
│   ├── Makefile                # Build system (MPLAB XC8)
│   ├── README.md               # Firmware documentation
│   └── .gitignore              # Build artifacts exclusion
│
├── python/                      # Raspberry Pi application
│   ├── main.py                 # Main GUI application
│   ├── plc_firmware.py         # PLC communication protocol
│   ├── jig.py                  # Jig control state machine
│   ├── hardware.py             # GPIO hardware abstraction
│   ├── uart.py                 # UART wrapper
│   ├── config.py               # Configuration loader
│   ├── logic.py                # QR validation logic
│   ├── settings.ini            # System configuration
│   ├── DEPLOYMENT_GUIDE.md     # Python deployment guide
│   └── ... (other modules)
│
├── INTEGRATION_GUIDE.md         # Firmware ↔ Python integration
├── QUICK_START.md               # Complete setup guide
├── README.md                    # This file
└── 20D0478_20201208_1146.pdf   # Hardware schematic
```

## Quick Links

- **[Quick Start Guide](QUICK_START.md)** - Get started from scratch
- **[Firmware Documentation](firmware/README.md)** - Build and program firmware
- **[Integration Guide](INTEGRATION_GUIDE.md)** - Firmware-Python integration details
- **[Python Deployment](python/DEPLOYMENT_GUIDE.md)** - Raspberry Pi setup

## Features

### Firmware (PIC18F4550)
- ✅ UART communication at 115200 baud
- ✅ GPIO handshaking with Raspberry Pi
- ✅ Pneumatic cylinder control (pusher, stopper)
- ✅ Sensor monitoring (safety, position, stack)
- ✅ LED indicators (Red, Green, Yellow)
- ✅ Buzzer feedback
- ✅ Button interface (Start, Stop, Reset)
- ✅ Retry logic (max 3 attempts)
- ✅ Safety interlock support
- ✅ Emergency stop functionality

### Python Application (Raspberry Pi)
- ✅ QR code scanning and validation
- ✅ Batch management and tracking
- ✅ Duplicate detection
- ✅ LCD display integration
- ✅ GUI interface (Tkinter)
- ✅ Database logging
- ✅ Configuration management
- ✅ Hardware abstraction layer

## Communication Protocol

### Commands (Firmware → Python)
- `0x14` - Scan request with retry
- `0x13` - Final scan attempt
- `0x00` - Stop/reset

### Responses (Python → Firmware)
- `A` - Accept (pass)
- `R` - Reject (fail)
- `D` - Duplicate
- `S` - Scanner error
- `Q` - No QR detected
- `L` - Length error

### Handshake
- **RASP_IN_PIC** (GPIO 18): Pi status signal (HIGH=ready, LOW=busy)
- Ensures synchronization between firmware and Python

## Hardware Requirements

### Microcontroller
- PIC18F4550 @ 48MHz (20MHz crystal + PLL)
- MPLAB XC8 compiler for firmware
- PICkit3/4 programmer

### Raspberry Pi
- Raspberry Pi 3B+ or 4 recommended
- Raspberry Pi OS (Raspbian)
- Python 3.7+
- UART enabled

### Jig Hardware
- Pneumatic cylinders and solenoid valves
- Position sensors (optical/proximity)
- Safety interlock switch
- LEDs (Red, Green, Yellow)
- Buzzer
- Push buttons (Start, Stop, Reset)
- QR code scanner/camera

## Getting Started

### 1. Build Firmware
```bash
cd firmware/
make all
# Programs hema_jig_firmware.hex
```

### 2. Program PIC18F4550
```bash
# Using PICkit
pk2cmd -P PIC18F4550 -F hema_jig_firmware.hex -M -R

# Or use MPLAB IPE (GUI)
```

### 3. Setup Raspberry Pi
```bash
# Enable UART in /boot/config.txt
# Install dependencies
sudo apt install python3-pip python3-tk python3-serial python3-rpi.gpio
pip3 install pyserial configparser

# Deploy application
cd python/
python3 main.py
```

### 4. Connect Hardware
- UART: Pi TX/RX ↔ PIC RC6/RC7
- GPIO: Pi GPIO 18 → PIC RB6 (RASP_IN_PIC)
- Power: 5V regulated for both devices
- See [QUICK_START.md](QUICK_START.md) for detailed wiring

### 5. Test System
```bash
# Test UART communication
python3 test_uart.py

# Test GPIO
python3 test_gpio.py

# Test integration
python3 test_integration.py
```

## Operation

1. **Load cartridges** into stack
2. **Press START** button on PIC controller
3. System automatically:
   - Advances cartridge to scanner
   - Requests QR scan from Pi
   - Validates against batch
   - Routes based on result (pass/reject)
   - Repeats for next cartridge
4. **Press STOP** to pause or RESET for emergency stop

## Documentation

| Document | Description |
|----------|-------------|
| [QUICK_START.md](QUICK_START.md) | Complete setup from scratch |
| [INTEGRATION_GUIDE.md](INTEGRATION_GUIDE.md) | Firmware ↔ Python integration |
| [firmware/README.md](firmware/README.md) | Firmware build and programming |
| [python/DEPLOYMENT_GUIDE.md](python/DEPLOYMENT_GUIDE.md) | Python application deployment |

## Troubleshooting

### No UART Communication
- Verify TX ↔ RX connections
- Check baud rate (115200)
- Enable UART on Pi (`/boot/config.txt`)

### GPIO Not Working
- Check GPIO 18 connection
- Verify Pi GPIO permissions
- Test manually with Python

### Pusher Not Moving
- Check air pressure (5-7 bar)
- Verify solenoid power
- Check safety interlock

See [QUICK_START.md](QUICK_START.md) for detailed troubleshooting.

## Safety

⚠️ **IMPORTANT SAFETY REQUIREMENTS:**

1. Safety interlock must be engaged for operation
2. Emergency stop must be accessible
3. Keep hands clear of moving parts
4. De-pressurize pneumatics before maintenance
5. Follow lockout/tagout procedures

## Development

### Building Firmware
Requires MPLAB XC8 Compiler (free version sufficient)
```bash
cd firmware/
make clean
make all
```

### Testing Python
```bash
cd python/
python3 -m pytest test_*.py
python3 test_integration.py
```

## License

Copyright (c) 2024 MOLBIO Diagnostics Limited  
Developed by QA Team Site-III

## Support

- **Firmware Issues**: See [firmware/README.md](firmware/README.md)
- **Python Issues**: See [python/DEPLOYMENT_GUIDE.md](python/DEPLOYMENT_GUIDE.md)
- **Integration Issues**: See [INTEGRATION_GUIDE.md](INTEGRATION_GUIDE.md)
- **Hardware Issues**: Check connections and sensors

## Version History

### v1.0.0 (Current)
- Initial release
- Complete firmware for PIC18F4550
- UART protocol implementation
- GPIO handshaking
- Jig mechanism control
- Safety features
- Comprehensive documentation

## Contributors

- QA Team Site-III, MOLBIO Diagnostics Limited
- Development based on existing Python application
- Firmware implementation for jig automation

---

For detailed information, see the guides in the documentation links above.
