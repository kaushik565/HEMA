/**
 * HEMA Jig Firmware - Main Program
 * PIC18F4550 Microcontroller
 * 
 * Automatic cartridge scanning jig controller
 * Communicates with Raspberry Pi via UART for QR code validation
 * 
 * Hardware:
 * - PIC18F4550 @ 48MHz
 * - UART: 115200 baud (RC6=TX, RC7=RX)
 * - GPIO handshake with Raspberry Pi (GPIO 18, 17, 27)
 * - Pneumatic cylinder control (pusher, stopper)
 * - Sensors (stack, position, safety)
 * - Indicator LEDs and buzzer
 * 
 * Protocol:
 * - PIC sends 0x14 (retry scan) or 0x13 (final scan) to request QR validation
 * - Pi responds with 'A' (accept), 'R' (reject), 'D' (duplicate), etc.
 * - RASP_IN_PIC line indicates Pi status (HIGH=ready, LOW=busy)
 */

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "pin_definitions.h"
#include "uart.h"
#include "jig_control.h"

// Configuration bits for PIC18F4550
#pragma config FOSC = HSPLL_HS      // HS oscillator, PLL enabled
#pragma config PLLDIV = 5           // Divide by 5 (20MHz / 5 = 4MHz for PLL input)
#pragma config CPUDIV = OSC1_PLL2   // CPU clock = PLL / 2 (96MHz / 2 = 48MHz)
#pragma config USBDIV = 2           // USB clock = 96MHz / 2 = 48MHz
#pragma config FCMEN = OFF          // Fail-safe clock monitor disabled
#pragma config IESO = OFF           // Internal/External switchover disabled
#pragma config PWRT = ON            // Power-up timer enabled
#pragma config BOR = ON             // Brown-out reset enabled
#pragma config BORV = 2             // Brown-out voltage 2.7V
#pragma config VREGEN = ON          // USB voltage regulator enabled
#pragma config WDT = OFF            // Watchdog timer disabled
#pragma config WDTPS = 32768        // Watchdog postscaler
#pragma config MCLRE = ON           // MCLR pin enabled
#pragma config LPT1OSC = OFF        // Timer1 low-power oscillator disabled
#pragma config PBADEN = OFF         // PORTB pins configured as digital on reset
#pragma config CCP2MX = ON          // CCP2 muxed with RC1
#pragma config STVREN = ON          // Stack overflow reset enabled
#pragma config LVP = OFF            // Low-voltage programming disabled
#pragma config ICPRT = OFF          // Dedicated ICPORT disabled
#pragma config XINST = OFF          // Extended instruction set disabled
#pragma config CP0 = OFF            // Code protection disabled
#pragma config CP1 = OFF
#pragma config CP2 = OFF
#pragma config CP3 = OFF
#pragma config CPB = OFF            // Boot block code protection
#pragma config CPD = OFF            // Data EEPROM code protection
#pragma config WRT0 = OFF           // Write protection disabled
#pragma config WRT1 = OFF
#pragma config WRT2 = OFF
#pragma config WRT3 = OFF
#pragma config WRTC = OFF           // Configuration register write protection
#pragma config WRTB = OFF           // Boot block write protection
#pragma config WRTD = OFF           // Data EEPROM write protection
#pragma config EBTR0 = OFF          // Table read protection disabled
#pragma config EBTR1 = OFF
#pragma config EBTR2 = OFF
#pragma config EBTR3 = OFF
#pragma config EBTRB = OFF          // Boot block table read protection

// Global variables
static volatile bool system_running = false;
static volatile bool emergency_stop = false;
static uint8_t scan_retry_count = 0;

// Function prototypes
void System_Init(void);
void Process_Buttons(void);
bool Wait_For_Pi_Ready(uint16_t timeout_ms);
uint8_t Request_QR_Scan(bool final_attempt);
void Handle_Scan_Response(uint8_t response);

/**
 * Main program entry point
 */
void main(void) {
    // Initialize system
    System_Init();
    
    // Main loop
    while (1) {
        // Check for emergency stop
        if (emergency_stop) {
            Jig_EmergencyStop();
            emergency_stop = false;
            system_running = false;
        }
        
        // Process button inputs
        Process_Buttons();
        
        // If system is running, execute jig operation
        if (system_running) {
            // Check safety first
            if (!Jig_IsSafetyOK()) {
                Jig_SetLEDs(false, true, false); // Red LED
                __delay_ms(100);
                continue;
            }
            
            // Check if cartridges available
            if (!Jig_IsStackPresent()) {
                Jig_SetLEDs(false, false, true); // Yellow LED (waiting)
                __delay_ms(100);
                continue;
            }
            
            // Advance one cartridge
            Jig_SetLEDs(false, false, true); // Yellow LED (working)
            
            // Extend pusher
            Jig_ExtendPusher();
            __delay_ms(PUSH_EXTEND_MS);
            
            // Retract pusher
            Jig_RetractPusher();
            __delay_ms(PUSH_RETRACT_MS);
            Jig_StopPusher();
            
            // Settle time
            __delay_ms(SETTLE_MS);
            
            // Wait for cartridge at scanner position
            uint16_t detect_timeout = DETECT_TIMEOUT_MS;
            while (!Jig_IsCartridgeAtScanner() && detect_timeout > 0) {
                __delay_ms(10);
                detect_timeout -= 10;
                
                // Check for stop button
                if (!BTN_STOP) {
                    system_running = false;
                    break;
                }
            }
            
            if (!system_running) {
                continue;
            }
            
            // Request QR scan from Raspberry Pi
            scan_retry_count = 0;
            bool scan_success = false;
            
            while (scan_retry_count < MAX_SCAN_RETRIES && !scan_success) {
                // Wait for Pi to be ready
                if (!Wait_For_Pi_Ready(2000)) {
                    Jig_SetLEDs(false, true, false); // Red LED (Pi not ready)
                    __delay_ms(500);
                    continue;
                }
                
                // Request scan (final attempt on last retry)
                bool final_attempt = (scan_retry_count == MAX_SCAN_RETRIES - 1);
                uint8_t response = Request_QR_Scan(final_attempt);
                
                // Handle response
                Handle_Scan_Response(response);
                
                if (response == RESP_ACCEPT) {
                    scan_success = true;
                } else if (response == RESP_REJECT || 
                          response == RESP_DUPLICATE ||
                          response == RESP_LENGTH_ERROR) {
                    scan_success = true; // Move to reject bin
                } else {
                    // Retry for errors or no QR
                    scan_retry_count++;
                }
            }
            
            // Brief pause before next cartridge
            __delay_ms(500);
        } else {
            // System idle - turn off LEDs
            Jig_SetLEDs(false, false, false);
            __delay_ms(100);
        }
    }
}

/**
 * Initialize system hardware and peripherals
 */
void System_Init(void) {
    // Initialize all pins
    INIT_ALL_PINS();
    
    // Initialize UART
    UART_Init();
    UART_Flush();
    
    // Initialize jig control
    Jig_Init();
    
    // Flash all LEDs on startup
    Jig_SetLEDs(true, true, true);
    __delay_ms(200);
    Jig_SetLEDs(false, false, false);
    
    // Short beep
    Jig_Buzz(100);
    
    // Wait for Pi to boot and signal ready
    for (uint8_t i = 0; i < 10; i++) {
        if (RASP_IN_PIC == 1) {
            Jig_SetLEDs(true, false, false); // Green LED (Pi ready)
            __delay_ms(500);
            Jig_SetLEDs(false, false, false);
            break;
        }
        __delay_ms(1000); // Wait 1 second
    }
    
    system_running = false;
    emergency_stop = false;
}

/**
 * Process button inputs with debouncing
 */
void Process_Buttons(void) {
    static uint8_t btn_start_state = 1;
    static uint8_t btn_stop_state = 1;
    static uint8_t btn_reset_state = 1;
    
    // Start button (active low with pull-up)
    if (BTN_START == 0 && btn_start_state == 1) {
        __delay_ms(DEBOUNCE_MS);
        if (BTN_START == 0) {
            // Start button pressed
            if (!system_running && Jig_IsSafetyOK()) {
                system_running = true;
                Jig_Buzz(50);
                Jig_SetLEDs(false, false, true); // Yellow LED
            }
            btn_start_state = 0;
        }
    } else if (BTN_START == 1) {
        btn_start_state = 1;
    }
    
    // Stop button (active low with pull-up)
    if (BTN_STOP == 0 && btn_stop_state == 1) {
        __delay_ms(DEBOUNCE_MS);
        if (BTN_STOP == 0) {
            // Stop button pressed
            system_running = false;
            Jig_Stop();
            Jig_Buzz(100);
            btn_stop_state = 0;
        }
    } else if (BTN_STOP == 1) {
        btn_stop_state = 1;
    }
    
    // Reset button (active low with pull-up)
    if (BTN_RESET == 0 && btn_reset_state == 1) {
        __delay_ms(DEBOUNCE_MS);
        if (BTN_RESET == 0) {
            // Reset button pressed (emergency stop)
            emergency_stop = true;
            btn_reset_state = 0;
        }
    } else if (BTN_RESET == 1) {
        btn_reset_state = 1;
    }
}

/**
 * Wait for Raspberry Pi ready signal (RASP_IN_PIC HIGH)
 */
bool Wait_For_Pi_Ready(uint16_t timeout_ms) {
    uint16_t elapsed = 0;
    
    while (RASP_IN_PIC == 0 && elapsed < timeout_ms) {
        __delay_ms(10);
        elapsed += 10;
    }
    
    return (RASP_IN_PIC == 1);
}

/**
 * Request QR scan from Raspberry Pi
 * Returns: Response byte from Pi ('A', 'R', 'D', 'S', etc.)
 */
uint8_t Request_QR_Scan(bool final_attempt) {
    uint8_t command = final_attempt ? CMD_SCAN_FINAL : CMD_SCAN_RETRY;
    uint8_t response = 0;
    
    // Flush UART receive buffer
    UART_Flush();
    
    // Send scan request
    UART_Write(command);
    
    // Wait for Pi to signal busy (RASP_IN_PIC LOW)
    uint16_t wait_count = 0;
    while (RASP_IN_PIC == 1 && wait_count < 500) {
        __delay_ms(10);
        wait_count += 10;
    }
    
    // Wait for response with timeout
    if (UART_ReadWithTimeout(&response, UART_TIMEOUT_MS)) {
        // Wait for Pi to signal ready again (RASP_IN_PIC HIGH)
        wait_count = 0;
        while (RASP_IN_PIC == 0 && wait_count < 1000) {
            __delay_ms(10);
            wait_count += 10;
        }
        
        return response;
    }
    
    // Timeout - return scanner error
    return RESP_SCANNER_ERROR;
}

/**
 * Handle scan response from Raspberry Pi
 */
void Handle_Scan_Response(uint8_t response) {
    switch (response) {
        case RESP_ACCEPT:
            // Pass - green LED and short beep
            Jig_SetLEDs(true, false, false);
            Jig_Buzz(100);
            break;
            
        case RESP_REJECT:
        case RESP_LENGTH_ERROR:
            // Reject - red LED and longer beep
            Jig_SetLEDs(false, true, false);
            Jig_Buzz(200);
            break;
            
        case RESP_DUPLICATE:
            // Duplicate - yellow LED and two beeps
            Jig_SetLEDs(false, false, true);
            Jig_Buzz(100);
            __delay_ms(100);
            Jig_Buzz(100);
            break;
            
        case RESP_SCANNER_ERROR:
        case RESP_HW_ERROR:
            // Error - flash red LED
            for (uint8_t i = 0; i < 3; i++) {
                Jig_SetLEDs(false, true, false);
                __delay_ms(100);
                Jig_SetLEDs(false, false, false);
                __delay_ms(100);
            }
            break;
            
        case RESP_NO_QR:
            // No QR detected - will retry
            Jig_SetLEDs(false, false, true);
            __delay_ms(300);
            break;
            
        default:
            // Unknown response
            Jig_SetLEDs(false, true, true);
            __delay_ms(200);
            break;
    }
}
