# Firmware-Python Integration Guide

This document explains how the PIC18F4550 firmware integrates with the Raspberry Pi Python application.

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                        System Architecture                       │
└─────────────────────────────────────────────────────────────────┘

┌──────────────────────┐              ┌──────────────────────┐
│  PIC18F4550 Firmware │              │   Raspberry Pi       │
│  (Jig Controller)    │◄────UART────►│   Python Application │
│                      │   115200 bps  │                      │
└──────────────────────┘              └──────────────────────┘
         │                                     │
         │ GPIO Handshake                     │
         ├── RB6 ◄─── GPIO 18 (RASP_IN_PIC)  │
         ├── RB5 ───► GPIO 17 (INT_PIC)      │
         └── RB7 ───► GPIO 27 (SHD_PIC)      │
         │                                     │
         │ Control                             │
         ├── Pusher Cylinder                  │
         ├── Stopper Mechanism                │
         ├── LEDs (R/G/Y)                     │
         └── Buzzer                            │
         │                                     │
         │ Sensors                             │
         ├── Safety Interlock                 │
         ├── Stack Present                    │
         ├── At Scanner Position              │
         └── Pusher Position                  │
```

## Communication Protocol Mapping

### Firmware → Python

| Firmware Code | Constant | Python Handler | Description |
|--------------|----------|----------------|-------------|
| `0x14` | `CMD_SCAN_RETRY` | `plc_firmware.CMD_RETRY` | Request scan with retry |
| `0x13` | `CMD_SCAN_FINAL` | `plc_firmware.CMD_FINAL` | Final scan attempt |
| `0x00` | `CMD_STOP` | N/A | Stop/reset command |

**Firmware Implementation:**
```c
// In main.c
uint8_t command = final_attempt ? CMD_SCAN_FINAL : CMD_SCAN_RETRY;
UART_Write(command);
```

**Python Handler:**
```python
# In plc_firmware.py - PLCHandshake class
def _handle_firmware_response(self, code: int) -> None:
    if code in (CMD_RETRY, CMD_FINAL):
        self._handle_scan_command(final_attempt=(code == CMD_FINAL))
```

### Python → Firmware

| Python Code | Constant | Firmware Handler | Description |
|------------|----------|-----------------|-------------|
| `'A'` | `RESP_ACCEPT` | Accept | Valid QR, pass |
| `'R'` | `RESP_REJECT` | Reject | Invalid QR, reject |
| `'D'` | `RESP_DUPLICATE` | Duplicate | Already scanned |
| `'S'` | `RESP_SCANNER_ERROR` | Error | Scanner error |
| `'Q'` | `RESP_NO_QR` | Retry | No QR detected |
| `'L'` | `RESP_LENGTH_ERROR` | Reject | Wrong length |
| `'B'` | `RESP_LOG_ERROR` | Error | Logging error |
| `'C'` | `RESP_REPEAT_TEST` | Error | Repeat test |
| `'H'` | `RESP_HW_ERROR` | Error | Hardware error |

**Python Implementation:**
```python
# In plc_firmware.py - PLCHandshake class
def _map_status(self, status: str) -> str:
    normalized = (status or "").upper()
    if normalized == "PASS":
        return "A"  # Accept
    if normalized == "DUPLICATE":
        return "D"  # Duplicate
    if normalized in {"INVALID FORMAT", "LINE MISMATCH", "OUT OF BATCH"}:
        return "R"  # Reject
    return "S"  # Scanner error
```

**Firmware Handler:**
```c
// In main.c
void Handle_Scan_Response(uint8_t response) {
    switch (response) {
        case RESP_ACCEPT:
            Jig_SetLEDs(true, false, false);  // Green
            Jig_Buzz(100);
            break;
        case RESP_REJECT:
        case RESP_LENGTH_ERROR:
            Jig_SetLEDs(false, true, false);  // Red
            Jig_Buzz(200);
            break;
        // ... other cases
    }
}
```

## GPIO Handshake Protocol

### RASP_IN_PIC (RB6 ← GPIO 18)

**Purpose:** Pi signals its status to the firmware

| State | Level | Meaning | Python Code | Firmware Check |
|-------|-------|---------|-------------|----------------|
| Ready | HIGH | Pi ready for commands | `hardware.set_busy(False)` | `RASP_IN_PIC == 1` |
| Busy | LOW | Pi processing scan | `hardware.set_busy(True)` | `RASP_IN_PIC == 0` |

**Python Implementation:**
```python
# In plc_firmware.py
def _set_busy(self, busy: bool) -> None:
    self._hardware.set_busy(busy)  # Sets GPIO 18

# In hardware.py - GPIOHardwareController
def set_busy(self, busy: bool) -> None:
    GPIO.output(self.busy_pin, GPIO.HIGH if busy else GPIO.LOW)
```

**Firmware Implementation:**
```c
// In main.c
bool Wait_For_Pi_Ready(uint16_t timeout_ms) {
    uint16_t elapsed = 0;
    while (RASP_IN_PIC == 0 && elapsed < timeout_ms) {
        __delay_ms(10);
        elapsed += 10;
    }
    return (RASP_IN_PIC == 1);
}
```

### INT_PIC (RB5 → GPIO 17)

**Purpose:** Firmware can signal interrupts to Pi (currently unused but available)

**Firmware:**
```c
// In pin_definitions.h
#define INT_PIC LATBbits.LATB5
```

### SHD_PIC (RB7 → GPIO 27)

**Purpose:** Shutdown signal (currently unused but available for future use)

**Firmware:**
```c
// In pin_definitions.h
#define SHD_PIC PORTBbits.RB7
```

## Sequence Diagrams

### Successful Scan Sequence

```
Firmware                 UART                Raspberry Pi
   │                      │                       │
   │─── 0x14 (RETRY) ────►│──────────────────────►│
   │                      │                       │ Process QR
   │                      │                       │ Validate
   │                      │                       │
   │                      │   RASP_IN_PIC LOW ◄───┤ (Busy)
   │                      │                       │
   │◄──────────────────────────── 'A' ────────────┤ Accept
   │                      │                       │
   │                      │   RASP_IN_PIC HIGH ◄──┤ (Ready)
   │                      │                       │
   │ Green LED ON         │                       │
   │ Beep (100ms)         │                       │
   │ Advance cartridge    │                       │
   │                      │                       │
```

### Retry Sequence (No QR)

```
Firmware                 UART                Raspberry Pi
   │                      │                       │
   │─── 0x14 (RETRY) ────►│──────────────────────►│
   │                      │                       │ Scan - No QR
   │                      │   RASP_IN_PIC LOW ◄───┤
   │◄──────────────────────────── 'Q' ────────────┤ No QR
   │                      │   RASP_IN_PIC HIGH ◄──┤
   │ Yellow LED           │                       │
   │                      │                       │
   │─── 0x14 (RETRY) ────►│──────────────────────►│ Retry #2
   │                      │                       │ Scan - Success
   │                      │   RASP_IN_PIC LOW ◄───┤
   │◄──────────────────────────── 'A' ────────────┤ Accept
   │                      │   RASP_IN_PIC HIGH ◄──┤
   │ Green LED            │                       │
   │ Advance              │                       │
```

### Final Attempt Sequence

```
Firmware                 UART                Raspberry Pi
   │                      │                       │
   │ (After 2 retries)    │                       │
   │─── 0x13 (FINAL) ────►│──────────────────────►│
   │                      │                       │ Final scan
   │                      │   RASP_IN_PIC LOW ◄───┤
   │◄──────────────────────────── 'Q' ────────────┤ Still no QR
   │                      │   RASP_IN_PIC HIGH ◄──┤
   │ Red LED              │                       │
   │ Beep (200ms)         │                       │
   │ Advance to reject    │                       │
```

## Configuration Mapping

### Firmware (config.h)

```c
#define UART_BAUDRATE 115200
#define UART_TIMEOUT_MS 12000
#define PUSH_EXTEND_MS 400
#define PUSH_RETRACT_MS 400
#define SETTLE_MS 200
#define DETECT_TIMEOUT_MS 3000
#define SCAN_TIMEOUT_MS 5000
#define MAX_SCAN_RETRIES 3
```

### Python (config.py - settings.ini defaults)

```python
"jig": {
    "push_extend_ms": "400",
    "push_retract_ms": "400",
    "settle_ms": "200",
    "detect_timeout_ms": "3000",
    "scan_timeout_ms": "5000",
}

"actj_legacy": {
    "uart_port": "/dev/serial0",
    "baudrate": "115200",
    "command_timeout": "12000",
}
```

**Important:** Keep these values synchronized between firmware and Python configuration!

## Testing Integration

### 1. Hardware Loopback Test

Connect UART TX to RX (loopback) to test UART functionality:

**Firmware:**
```c
// In main.c, add to System_Init()
UART_Write('T');  // Send test byte
__delay_ms(100);
if (UART_DataAvailable()) {
    uint8_t echo = UART_Read();
    if (echo == 'T') {
        Jig_SetLEDs(true, false, false);  // Green = success
    }
}
```

### 2. Python → Firmware Test

**Python:**
```python
# Test script
import serial
ser = serial.Serial('/dev/serial0', 115200)
ser.write(b'A')  # Send accept
time.sleep(0.1)
print(f"Sent: A")
```

**Firmware:** Should show green LED and beep

### 3. Firmware → Python Test

**Python:**
```python
# Monitor commands from firmware
import serial
ser = serial.Serial('/dev/serial0', 115200)
while True:
    if ser.in_waiting:
        cmd = ser.read(1)
        print(f"Received: 0x{cmd[0]:02X}")
```

**Firmware:** Press START button, should see `0x14` commands

### 4. Full Integration Test

Run the Python test integration:

```bash
cd /home/runner/work/HEMA/HEMA/python
python3 test_integration.py
```

This tests:
- UART communication
- Command/response protocol
- GPIO handshaking
- Timeout handling

## Troubleshooting Integration Issues

### Symptom: No UART Communication

**Check:**
1. Physical connections (TX ↔ RX, common GND)
2. Baud rate match (115200 on both sides)
3. Pi UART enabled (`/boot/config.txt`)
4. Firmware UART initialized (`UART_Init()` called)

**Debug:**
```python
# Python side
import serial
ser = serial.Serial('/dev/serial0', 115200)
ser.write(b'\x14')  # Send scan command
```

### Symptom: Pi Not Signaling Ready

**Check:**
1. GPIO 18 connection (Pi → RB6)
2. Python hardware controller initialized
3. `set_busy()` called correctly

**Debug:**
```python
# Force GPIO 18 HIGH
import RPi.GPIO as GPIO
GPIO.setmode(GPIO.BCM)
GPIO.setup(18, GPIO.OUT)
GPIO.output(18, GPIO.HIGH)
```

### Symptom: Firmware Not Responding to Pi

**Check:**
1. Response timeout (12 seconds default)
2. UART receive working (`UART_Read()`)
3. Correct response codes sent

**Debug:**
```c
// Firmware: Echo received bytes
while (1) {
    if (UART_DataAvailable()) {
        uint8_t data = UART_Read();
        UART_Write(data);  // Echo back
    }
}
```

## Performance Optimization

### Reduce Latency

**Firmware:**
```c
// Reduce polling delays
#define POLL_DELAY_MS 5  // Instead of 10ms
```

**Python:**
```python
# Increase poll frequency
self._poll_interval_ms = 10  # Instead of 20ms
```

### Increase Throughput

**Firmware:**
```c
// Reduce mechanical delays (if safe)
#define PUSH_EXTEND_MS 300   // From 400ms
#define PUSH_RETRACT_MS 300  // From 400ms
```

## Safety Considerations

1. **Always check SENSOR_SAFETY_OK** before any movement
2. **De-energize actuators** on emergency stop
3. **Implement timeout on all waits** to prevent lockups
4. **Validate all UART data** before processing
5. **Keep handshake synchronized** to prevent race conditions

## Additional Resources

- **Firmware README**: `/firmware/README.md`
- **Python Deployment Guide**: `/python/DEPLOYMENT_GUIDE.md`
- **Hardware Abstraction**: `/python/hardware.py`
- **PLC Communication**: `/python/plc_firmware.py`
- **Jig Control**: `/python/jig.py`

## Version Compatibility

| Firmware Version | Python Version | Notes |
|-----------------|----------------|-------|
| v1.0.0 | All versions | Initial release |

Ensure firmware and Python versions are compatible for proper operation.
