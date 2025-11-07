# HEMA - QR Scanner Testing Jig

This repository contains firmware and control software for an automated QR scanner testing jig system.

## Project Structure

```
new_qr_firmware/
├── python/           # Raspberry Pi control software (Python)
│   ├── main.py       # Main application
│   ├── config.py     # Configuration
│   ├── hardware.py   # Hardware interface
│   ├── logic.py      # Business logic
│   └── ...           # Other modules
├── *.c, *.h          # PIC18F4550 firmware (C)
├── qr_clean/         # Clean firmware build
└── old firmware/     # Legacy firmware versions
```

## Components

### PIC18F4550 Firmware (C)
- Microcontroller firmware for the testing jig hardware
- Controls motors, sensors, and LCD display
- Communicates with Raspberry Pi via UART
- **Build Requirements**: MPLAB C18 v3.47 (Windows)
- See `qr_clean/build.bat` for build instructions

### Raspberry Pi Software (Python 3)
- GUI application for test management
- Communicates with PIC firmware via UART
- Logs test results to CSV files
- **Dependencies**: pyserial, flask, tkinter, RPi.GPIO

## Building

### C Firmware
The PIC firmware requires MPLAB C18 compiler (Windows only):
```bash
cd new_qr_firmware/qr_clean
build.bat
```

### Python Software
The Python code runs on Raspberry Pi:
```bash
cd new_qr_firmware/python
pip install pyserial flask
python3 main.py
```

## CI/CD

The GitHub Actions workflow validates:
- Python syntax and imports
- C source file presence
- Code structure integrity

Note: Actual firmware compilation requires proprietary tools and is not performed in CI.

## Hardware

- **Microcontroller**: PIC18F4550
- **Computer**: Raspberry Pi
- **Interface**: UART @ 115200 baud
- **Display**: I2C LCD
- **Sensors**: GPIO-based limit switches and sensors
