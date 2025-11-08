/**
 * Configuration Header for HEMA Jig Firmware
 * PIC18F4550 Microcontroller
 * 
 * This file contains system configuration and constants for the jig controller
 */

#ifndef CONFIG_H
#define CONFIG_H

// System Configuration
#define _XTAL_FREQ 48000000  // 48MHz oscillator frequency

// UART Configuration
#define UART_BAUDRATE 115200
#define UART_TIMEOUT_MS 12000  // 12 second timeout for Raspberry Pi response

// Timing Constants (in milliseconds)
#define PUSH_EXTEND_MS 400      // Time for pusher to extend
#define PUSH_RETRACT_MS 400     // Time for pusher to retract
#define SETTLE_MS 200           // Settle time after pusher movement
#define DETECT_TIMEOUT_MS 3000  // Timeout for cartridge detection
#define SCAN_TIMEOUT_MS 5000    // Timeout for QR scan
#define DEBOUNCE_MS 50          // Button debounce time

// Protocol Commands (PIC -> Pi)
#define CMD_SCAN_RETRY 0x14     // Scan with retry (20 decimal)
#define CMD_SCAN_FINAL 0x13     // Final scan attempt (19 decimal)
#define CMD_STOP 0x00           // Stop/reset command

// Protocol Responses (Pi -> PIC)
#define RESP_ACCEPT 'A'         // Accept/Pass - advance to good bin
#define RESP_REJECT 'R'         // Reject/Fail - advance to reject bin
#define RESP_DUPLICATE 'D'      // Duplicate QR - advance to reject bin
#define RESP_SCANNER_ERROR 'S'  // Scanner error - retry or reject
#define RESP_NO_QR 'Q'          // No QR detected - retry
#define RESP_LENGTH_ERROR 'L'   // QR length error - reject
#define RESP_LOG_ERROR 'B'      // Logging error
#define RESP_REPEAT_TEST 'C'    // Repeated testing
#define RESP_HW_ERROR 'H'       // Hardware error

// Retry Configuration
#define MAX_SCAN_RETRIES 3      // Maximum scan retry attempts

// Mechanism Configuration
#define PUSHER_STEP_DELAY 10    // Delay between pusher control steps (ms)

#endif // CONFIG_H
