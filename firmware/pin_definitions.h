/**
 * Pin Definitions for HEMA Jig Firmware
 * PIC18F4550 Microcontroller
 * 
 * This file defines all I/O pin assignments for the jig controller
 */

#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H

#include <xc.h>

// ============================================================================
// UART Pins (RC6/RC7)
// ============================================================================
// RC6 - UART TX (to Raspberry Pi RX)
// RC7 - UART RX (from Raspberry Pi TX)

// ============================================================================
// Raspberry Pi Communication Pins
// ============================================================================
#define RASP_IN_PIC PORTBbits.RB6   // Status from Pi (GPIO 18)
                                     // HIGH = Pi ready, LOW = Pi busy
#define RASP_IN_PIC_TRIS TRISBbits.TRISB6

#define INT_PIC LATBbits.LATB5       // Interrupt to Pi (GPIO 17/24)
#define INT_PIC_TRIS TRISBbits.TRISB5

#define SHD_PIC PORTBbits.RB7        // Shutdown signal (GPIO 27/25)
#define SHD_PIC_TRIS TRISBbits.TRISB7

// ============================================================================
// Pusher Mechanism Control (Pneumatic Cylinder)
// ============================================================================
#define PUSHER_EXTEND LATAbits.LATA0     // Pusher extend solenoid
#define PUSHER_EXTEND_TRIS TRISAbits.TRISA0

#define PUSHER_RETRACT LATAbits.LATA1    // Pusher retract solenoid
#define PUSHER_RETRACT_TRIS TRISAbits.TRISA1

// ============================================================================
// Stopper Mechanism Control
// ============================================================================
#define STOPPER_UP LATAbits.LATA2        // Stopper up solenoid
#define STOPPER_UP_TRIS TRISAbits.TRISA2

#define STOPPER_DOWN LATAbits.LATA3      // Stopper down solenoid
#define STOPPER_DOWN_TRIS TRISAbits.TRISA3

// ============================================================================
// Sensors
// ============================================================================
#define SENSOR_STACK_PRESENT PORTBbits.RB0    // Cartridge stack present
#define SENSOR_STACK_PRESENT_TRIS TRISBbits.TRISB0

#define SENSOR_AT_SCANNER PORTBbits.RB1       // Cartridge at scanner position
#define SENSOR_AT_SCANNER_TRIS TRISBbits.TRISB1

#define SENSOR_PUSHER_EXTENDED PORTBbits.RB2  // Pusher fully extended
#define SENSOR_PUSHER_EXTENDED_TRIS TRISBbits.TRISB2

#define SENSOR_PUSHER_RETRACTED PORTBbits.RB3 // Pusher fully retracted
#define SENSOR_PUSHER_RETRACTED_TRIS TRISBbits.TRISB3

#define SENSOR_SAFETY_OK PORTBbits.RB4        // Safety interlock OK
#define SENSOR_SAFETY_OK_TRIS TRISBbits.TRISB4

// ============================================================================
// Indicator Lights (Optional - can be controlled by Pi or PIC)
// ============================================================================
#define LED_GREEN LATCbits.LATC0         // Green LED (Pass)
#define LED_GREEN_TRIS TRISCbits.TRISC0

#define LED_RED LATCbits.LATC1           // Red LED (Reject)
#define LED_RED_TRIS TRISCbits.TRISC1

#define LED_YELLOW LATCbits.LATC2        // Yellow LED (Duplicate/Warning)
#define LED_YELLOW_TRIS TRISCbits.TRISC2

// ============================================================================
// Buzzer
// ============================================================================
#define BUZZER LATDbits.LATD0           // Buzzer output
#define BUZZER_TRIS TRISDbits.TRISD0

// ============================================================================
// User Interface (Buttons)
// ============================================================================
#define BTN_START PORTEbits.RE0         // Start button
#define BTN_START_TRIS TRISEbits.TRISE0

#define BTN_STOP PORTEbits.RE1          // Stop button
#define BTN_STOP_TRIS TRISEbits.TRISE1

#define BTN_RESET PORTEbits.RE2         // Reset button
#define BTN_RESET_TRIS TRISEbits.TRISE2

// ============================================================================
// Pin Initialization Macros
// ============================================================================

// Initialize all digital I/O pins
#define INIT_ALL_PINS() do { \
    /* Configure analog pins as digital */ \
    ADCON1 = 0x0F; \
    \
    /* Raspberry Pi Communication */ \
    RASP_IN_PIC_TRIS = 1;  /* Input from Pi */ \
    INT_PIC_TRIS = 0;      /* Output to Pi */ \
    SHD_PIC_TRIS = 0;      /* Output */ \
    INT_PIC = 0;           /* Initialize low */ \
    SHD_PIC = 0;           /* Initialize low */ \
    \
    /* Pusher Mechanism */ \
    PUSHER_EXTEND_TRIS = 0;    /* Output */ \
    PUSHER_RETRACT_TRIS = 0;   /* Output */ \
    PUSHER_EXTEND = 0;         /* Initialize off */ \
    PUSHER_RETRACT = 0;        /* Initialize off */ \
    \
    /* Stopper Mechanism */ \
    STOPPER_UP_TRIS = 0;       /* Output */ \
    STOPPER_DOWN_TRIS = 0;     /* Output */ \
    STOPPER_UP = 0;            /* Initialize off */ \
    STOPPER_DOWN = 0;          /* Initialize off */ \
    \
    /* Sensors (all inputs with pull-ups) */ \
    SENSOR_STACK_PRESENT_TRIS = 1; \
    SENSOR_AT_SCANNER_TRIS = 1; \
    SENSOR_PUSHER_EXTENDED_TRIS = 1; \
    SENSOR_PUSHER_RETRACTED_TRIS = 1; \
    SENSOR_SAFETY_OK_TRIS = 1; \
    INTCON2bits.RBPU = 0;      /* Enable PORT B pull-ups */ \
    \
    /* Indicator LEDs */ \
    LED_GREEN_TRIS = 0;        /* Output */ \
    LED_RED_TRIS = 0;          /* Output */ \
    LED_YELLOW_TRIS = 0;       /* Output */ \
    LED_GREEN = 0; \
    LED_RED = 0; \
    LED_YELLOW = 0; \
    \
    /* Buzzer */ \
    BUZZER_TRIS = 0;           /* Output */ \
    BUZZER = 0;                /* Initialize off */ \
    \
    /* Buttons (inputs with pull-ups) */ \
    BTN_START_TRIS = 1; \
    BTN_STOP_TRIS = 1; \
    BTN_RESET_TRIS = 1; \
} while(0)

// De-energize all actuators (safety)
#define DEENERGIZE_ALL() do { \
    PUSHER_EXTEND = 0; \
    PUSHER_RETRACT = 0; \
    STOPPER_UP = 0; \
    STOPPER_DOWN = 0; \
    LED_GREEN = 0; \
    LED_RED = 0; \
    LED_YELLOW = 0; \
    BUZZER = 0; \
} while(0)

#endif // PIN_DEFINITIONS_H
