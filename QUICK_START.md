# HEMA Jig System - Quick Start Guide

Complete guide to get the automatic cartridge scanning jig operational from scratch.

## System Overview

The HEMA jig system consists of:
1. **PIC18F4550 Microcontroller** - Controls mechanical jig hardware
2. **Raspberry Pi** - Runs QR scanning and validation software
3. **Pneumatic Cylinders** - Pushes cartridges through the system
4. **Sensors** - Detects cartridge position and safety status
5. **QR Scanner** - Reads cartridge identification codes

## Prerequisites

### Hardware
- [ ] PIC18F4550 microcontroller with 20MHz crystal
- [ ] PICkit3/PICkit4 programmer
- [ ] Raspberry Pi (3B+ or 4 recommended)
- [ ] UART serial cable (Pi to PIC)
- [ ] Power supply (5V regulated for both devices)
- [ ] Pneumatic air supply and solenoid valves
- [ ] Position sensors (optical/proximity)
- [ ] Safety interlock switch
- [ ] LEDs (Red, Green, Yellow)
- [ ] Buzzer
- [ ] Push buttons (Start, Stop, Reset)

### Software
- [ ] MPLAB XC8 Compiler (for firmware)
- [ ] PICkit programming software
- [ ] Raspberry Pi OS (Raspbian)
- [ ] Python 3.7 or higher

## Step-by-Step Setup

### Part 1: Build and Program Firmware

#### 1.1 Install MPLAB XC8 Compiler

**Windows:**
```
1. Download from: https://www.microchip.com/mplab/compilers
2. Run installer (free version is sufficient)
3. Add to PATH: C:\Program Files\Microchip\xc8\vX.XX\bin
```

**Linux:**
```bash
# Download installer
wget https://www.microchip.com/mymicrochip/filehandler.aspx?ddocname=en589765

# Install
chmod +x xc8-vX.XX-linux.run
sudo ./xc8-vX.XX-linux.run

# Add to PATH
export PATH=/opt/microchip/xc8/vX.XX/bin:$PATH
```

#### 1.2 Build Firmware

```bash
cd firmware/
make clean
make all
```

Expected output:
```
Building firmware...
Compiling main.c...
Compiling uart.c...
Compiling jig_control.c...
Linking...
Build complete: hema_jig_firmware.hex
```

#### 1.3 Program PIC18F4550

**Using MPLAB IPE:**
```
1. Connect PICkit to PC and PIC
2. Launch MPLAB IPE
3. Device: PIC18F4550
4. Load hema_jig_firmware.hex
5. Click "Program"
6. Verify successful
```

**Using Command Line:**
```bash
pk2cmd -P PIC18F4550 -F hema_jig_firmware.hex -M -R
```

### Part 2: Setup Raspberry Pi

#### 2.1 Install Raspberry Pi OS

```bash
# Download Raspberry Pi Imager
# Flash SD card with Raspberry Pi OS Lite or Desktop
# Boot Pi and complete initial setup
```

#### 2.2 Enable UART

Edit boot configuration:
```bash
sudo nano /boot/config.txt
```

Add these lines:
```
enable_uart=1
dtparam=uart=on
```

Disable serial console:
```bash
sudo nano /boot/cmdline.txt
```

Remove: `console=serial0,115200`

Reboot:
```bash
sudo reboot
```

#### 2.3 Install Dependencies

```bash
sudo apt update
sudo apt upgrade -y

# Install Python packages
sudo apt install -y python3-pip python3-tk python3-serial python3-rpi.gpio

# Install Python modules
pip3 install pyserial configparser
```

#### 2.4 Configure GPIO Permissions

```bash
sudo usermod -a -G gpio pi
sudo chmod 666 /dev/gpiomem
```

#### 2.5 Deploy Python Application

```bash
# Create application directory
mkdir -p /home/pi/hema
cd /home/pi/hema

# Copy Python files from repository
# (Transfer via USB, SCP, or git clone)
git clone https://github.com/kaushik565/HEMA.git
cd HEMA/python

# Test UART
ls -la /dev/serial0
# Should show: lrwxrwxrwx 1 root root ... /dev/serial0 -> ttyAMA0
```

### Part 3: Hardware Connections

#### 3.1 UART Connection (Pi ↔ PIC)

```
Raspberry Pi          PIC18F4550
━━━━━━━━━━━━          ━━━━━━━━━━
Pin 8 (GPIO 14) TX  → RC7 (Pin 26) RX
Pin 10 (GPIO 15) RX ← RC6 (Pin 25) TX
Pin 6 (GND)         ↔ VSS (Pin 12, 31)
```

#### 3.2 GPIO Handshake (Pi ↔ PIC)

```
Raspberry Pi          PIC18F4550
━━━━━━━━━━━━          ━━━━━━━━━━
Pin 12 (GPIO 18)    → RB6 (Pin 39) RASP_IN_PIC
Pin 11 (GPIO 17)    ← RB5 (Pin 38) INT_PIC (optional)
Pin 13 (GPIO 27)    ← RB7 (Pin 40) SHD_PIC (optional)
```

#### 3.3 Verify Connections

```bash
# Test GPIO
python3 test_gpio.py

# Test UART loopback (connect TX to RX)
python3 test_uart.py
```

### Part 4: Configuration

#### 4.1 Edit settings.ini

```bash
cd /home/pi/hema/HEMA/python
nano settings.ini
```

Key sections:
```ini
[hardware]
controller = gpio          # Use 'gpio' for production, 'mock' for testing
pin_mode = BCM
red_pin = 20
green_pin = 21
yellow_pin = 22
buzzer_pin = 23

[actj_legacy]
enabled = true
uart_port = /dev/serial0
baudrate = 115200
gpio_rasp_in_pic = 18
command_timeout = 12000

[jig]
enabled = false            # Set true if using jig.py automation
push_extend_ms = 400
push_retract_ms = 400
settle_ms = 200
```

#### 4.2 Verify Firmware Configuration

Ensure firmware `config.h` matches:
```c
#define UART_BAUDRATE 115200
#define PUSH_EXTEND_MS 400
#define PUSH_RETRACT_MS 400
#define SETTLE_MS 200
```

### Part 5: Testing

#### 5.1 Test Communication

**Terminal 1 (Pi):**
```bash
cd /home/pi/hema/HEMA/python
python3 test_integration.py
```

**Terminal 2 (Monitor):**
```bash
# Watch for communication
minicom -D /dev/serial0 -b 115200
```

#### 5.2 Test Firmware Buttons

1. Press **START** button on PIC board
2. Should see:
   - Yellow LED on
   - Pusher extend/retract
   - UART command (0x14) sent to Pi

3. Press **STOP** button
   - System should halt
   - LEDs off

4. Press **RESET** button
   - Emergency stop
   - All actuators de-energized
   - Red LED flash

#### 5.3 Test Python Application

```bash
cd /home/pi/hema/HEMA/python
python3 main.py
```

Expected:
```
INFO:hardware:Using GPIO hardware controller
INFO:plc.handshake:Linked to PLC controller on /dev/serial0
INFO:main:Application started
```

### Part 6: First Operational Run

#### 6.1 Setup Batch

1. Launch application:
   ```bash
   python3 main.py
   ```

2. Enter batch information:
   - Batch Number: (10 characters)
   - Batch Line: (A, B, C, etc.)
   - Mould Range: Start and end QR codes

3. Click "Start Scanning"

#### 6.2 Load Cartridges

1. Check safety interlock engaged
2. Load cartridges into stack
3. Press **START** button on PIC controller

#### 6.3 Observe Operation

Watch for:
- [ ] Pusher advances cartridge
- [ ] Cartridge detected at scanner
- [ ] QR code scanned (camera/manual)
- [ ] Validation result displayed
- [ ] Appropriate LED lights (Green=Pass, Red=Reject)
- [ ] Buzzer feedback
- [ ] Next cartridge advances

#### 6.4 Stop Operation

Press **STOP** button on PIC or click "Stop" in GUI

## Troubleshooting

### Issue: No UART Communication

**Symptoms:** No data between Pi and PIC

**Check:**
```bash
# Verify UART enabled
ls -la /dev/serial0

# Test with loopback
# Connect Pi TX (Pin 8) to Pi RX (Pin 10)
echo "test" > /dev/serial0
cat /dev/serial0

# Check PIC power
# Measure 5V on VDD pins

# Verify baud rate
# Should be 115200 on both sides
```

### Issue: GPIO Not Working

**Symptoms:** RASP_IN_PIC not responding

**Check:**
```bash
# Test GPIO manually
python3
>>> import RPi.GPIO as GPIO
>>> GPIO.setmode(GPIO.BCM)
>>> GPIO.setup(18, GPIO.OUT)
>>> GPIO.output(18, GPIO.HIGH)
# Measure voltage on RB6 - should be 3.3V

>>> GPIO.output(18, GPIO.LOW)
# Should be 0V
```

### Issue: Pusher Not Moving

**Symptoms:** No pneumatic movement

**Check:**
- Air supply pressure (typically 5-7 bar)
- Solenoid valve power (12V or 24V)
- Wiring from PIC (RA0, RA1) to valves
- Relay/driver circuit if used
- Safety interlock (must be engaged)

### Issue: Sensors Not Detecting

**Symptoms:** Cartridge position not detected

**Check:**
- Sensor power supply (typically 12V or 24V)
- Sensor output type (NPN/PNP)
- Pull-up resistors on PIC inputs (enabled)
- Sensor alignment and distance
- Wiring to PIC (RB0-RB4)

### Issue: Python Application Crashes

**Check logs:**
```bash
cd /home/pi/hema/HEMA/python
ls -la batch_logs/
tail -f batch_logs/jig.log
```

**Common fixes:**
```bash
# Reinstall dependencies
pip3 install --upgrade pyserial

# Check permissions
sudo usermod -a -G dialout pi
sudo usermod -a -G gpio pi

# Reboot
sudo reboot
```

## Production Deployment Checklist

- [ ] Firmware programmed and verified
- [ ] Raspberry Pi configured with correct settings
- [ ] UART communication tested and working
- [ ] GPIO handshake verified
- [ ] All sensors calibrated and tested
- [ ] Pneumatic system pressurized and leak-free
- [ ] Safety interlock functional
- [ ] Emergency stop tested
- [ ] LED indicators working
- [ ] Buzzer functioning
- [ ] Batch setup completed
- [ ] Test run with sample cartridges successful
- [ ] Operators trained on system
- [ ] Maintenance schedule established

## Maintenance

### Daily
- Check air pressure
- Verify sensor operation
- Test emergency stop
- Clear any jammed cartridges

### Weekly
- Clean sensors
- Check connections
- Verify QR scanner focus
- Review error logs

### Monthly
- Lubricate moving parts
- Check pneumatic fittings for leaks
- Verify calibration
- Backup configuration

## Safety Guidelines

⚠️ **CRITICAL SAFETY RULES:**

1. **Always engage safety interlock** before operation
2. **Keep hands clear** of moving parts during operation
3. **Emergency stop must be accessible** at all times
4. **De-pressurize pneumatics** before maintenance
5. **Disconnect power** before wiring changes
6. **Test emergency stop** before each shift
7. **Wear safety glasses** when near jig
8. **Follow lockout/tagout procedures** for maintenance

## Support

### Documentation
- Firmware: `/firmware/README.md`
- Python: `/python/DEPLOYMENT_GUIDE.md`
- Integration: `/INTEGRATION_GUIDE.md`

### Logs
- Application: `/home/pi/hema/HEMA/python/batch_logs/jig.log`
- System: `/var/log/syslog`
- UART: Use `minicom -D /dev/serial0 -b 115200`

### Testing
```bash
# GPIO test
python3 test_gpio.py

# UART test
python3 test_uart.py

# Integration test
python3 test_integration.py
```

### Contact
- Development Team: QA Team Site-III
- Company: MOLBIO Diagnostics Limited

---

## Next Steps After Setup

1. **Operator Training**: Train personnel on system operation
2. **Validation Runs**: Perform qualification runs with known samples
3. **Documentation**: Complete as-built documentation
4. **Backup**: Create backup of working configuration
5. **Monitoring**: Set up logging and monitoring systems

## Congratulations!

Your HEMA automatic cartridge scanning jig is now operational. Follow proper operating procedures and maintenance schedules for reliable performance.

For any issues not covered in this guide, consult the detailed documentation in `/firmware/` and `/python/` directories.
