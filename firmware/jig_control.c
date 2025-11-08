/**
 * Jig Mechanism Control Module Implementation
 * PIC18F4550 Microcontroller
 * 
 * State machine for cartridge handling automation
 */

#include "jig_control.h"
#include "pin_definitions.h"
#include "config.h"
#include <xc.h>

// Internal state
static JigState_t current_state = JIG_STATE_IDLE;
static bool jig_running = false;
static uint16_t state_timer = 0;

/**
 * Initialize jig control module
 */
void Jig_Init(void) {
    current_state = JIG_STATE_IDLE;
    jig_running = false;
    state_timer = 0;
    
    // Ensure all actuators are de-energized
    DEENERGIZE_ALL();
}

/**
 * Start jig operation
 */
void Jig_Start(void) {
    if (Jig_IsSafetyOK()) {
        jig_running = true;
        current_state = JIG_STATE_CHECK_STACK;
        Jig_SetLEDs(false, false, true); // Yellow LED on (running)
    }
}

/**
 * Stop jig operation
 */
void Jig_Stop(void) {
    jig_running = false;
    current_state = JIG_STATE_IDLE;
    DEENERGIZE_ALL();
}

/**
 * Check safety interlock status
 */
bool Jig_IsSafetyOK(void) {
    // Active low sensor - return true if sensor is grounded (safety OK)
    return (SENSOR_SAFETY_OK == 0);
}

/**
 * Check if cartridge stack is present
 */
bool Jig_IsStackPresent(void) {
    // Active low sensor
    return (SENSOR_STACK_PRESENT == 0);
}

/**
 * Check if cartridge is at scanner position
 */
bool Jig_IsCartridgeAtScanner(void) {
    // Active low sensor
    return (SENSOR_AT_SCANNER == 0);
}

/**
 * Extend pusher cylinder
 */
void Jig_ExtendPusher(void) {
    PUSHER_RETRACT = 0;
    __delay_ms(PUSHER_STEP_DELAY);
    PUSHER_EXTEND = 1;
}

/**
 * Retract pusher cylinder
 */
void Jig_RetractPusher(void) {
    PUSHER_EXTEND = 0;
    __delay_ms(PUSHER_STEP_DELAY);
    PUSHER_RETRACT = 1;
}

/**
 * Stop pusher movement
 */
void Jig_StopPusher(void) {
    PUSHER_EXTEND = 0;
    PUSHER_RETRACT = 0;
}

/**
 * Raise stopper
 */
void Jig_RaiseStoropper(void) {
    STOPPER_DOWN = 0;
    __delay_ms(PUSHER_STEP_DELAY);
    STOPPER_UP = 1;
}

/**
 * Lower stopper
 */
void Jig_LowerStopper(void) {
    STOPPER_UP = 0;
    __delay_ms(PUSHER_STEP_DELAY);
    STOPPER_DOWN = 1;
}

/**
 * Stop stopper movement
 */
void Jig_StopStopper(void) {
    STOPPER_UP = 0;
    STOPPER_DOWN = 0;
}

/**
 * Control indicator LEDs
 */
void Jig_SetLEDs(bool green, bool red, bool yellow) {
    LED_GREEN = green ? 1 : 0;
    LED_RED = red ? 1 : 0;
    LED_YELLOW = yellow ? 1 : 0;
}

/**
 * Sound buzzer for specified duration
 */
void Jig_Buzz(uint16_t duration_ms) {
    BUZZER = 1;
    for (uint16_t i = 0; i < duration_ms; i++) {
        __delay_ms(1);
    }
    BUZZER = 0;
}

/**
 * Emergency stop - de-energize all actuators immediately
 */
void Jig_EmergencyStop(void) {
    DEENERGIZE_ALL();
    jig_running = false;
    current_state = JIG_STATE_ERROR;
    
    // Flash red LED as warning
    for (uint8_t i = 0; i < 5; i++) {
        LED_RED = 1;
        __delay_ms(100);
        LED_RED = 0;
        __delay_ms(100);
    }
}

/**
 * Main state machine - called periodically from main loop
 * Note: This is a simplified version. Full implementation would be
 * coordinated with the Raspberry Pi via UART in main.c
 */
void Jig_ProcessStateMachine(void) {
    if (!jig_running) {
        current_state = JIG_STATE_IDLE;
        return;
    }
    
    switch (current_state) {
        case JIG_STATE_IDLE:
            // Waiting for start command
            DEENERGIZE_ALL();
            break;
            
        case JIG_STATE_WAIT_SAFETY:
            // Wait for safety interlock
            if (Jig_IsSafetyOK()) {
                current_state = JIG_STATE_CHECK_STACK;
            } else {
                Jig_SetLEDs(false, true, false); // Red LED (safety error)
            }
            break;
            
        case JIG_STATE_CHECK_STACK:
            // Check if cartridges available
            if (!Jig_IsSafetyOK()) {
                current_state = JIG_STATE_WAIT_SAFETY;
            } else if (Jig_IsStackPresent()) {
                current_state = JIG_STATE_PUSH_EXTEND;
            } else {
                Jig_SetLEDs(false, false, true); // Yellow LED (waiting for stack)
            }
            break;
            
        case JIG_STATE_PUSH_EXTEND:
            // Extend pusher to advance cartridge
            Jig_ExtendPusher();
            __delay_ms(PUSH_EXTEND_MS);
            current_state = JIG_STATE_PUSH_RETRACT;
            break;
            
        case JIG_STATE_PUSH_RETRACT:
            // Retract pusher
            Jig_RetractPusher();
            __delay_ms(PUSH_RETRACT_MS);
            Jig_StopPusher();
            __delay_ms(SETTLE_MS);
            current_state = JIG_STATE_WAIT_DETECTION;
            break;
            
        case JIG_STATE_WAIT_DETECTION:
            // Wait for cartridge at scanner position
            // This would normally have a timeout
            if (Jig_IsCartridgeAtScanner()) {
                current_state = JIG_STATE_WAIT_SCAN;
            }
            break;
            
        case JIG_STATE_WAIT_SCAN:
            // Waiting for scan result from Raspberry Pi
            // This state is handled by UART communication in main.c
            Jig_SetLEDs(false, false, true); // Yellow LED (scanning)
            break;
            
        case JIG_STATE_ADVANCE_GOOD:
            // Advance to good bin
            Jig_SetLEDs(true, false, false); // Green LED
            Jig_Buzz(100); // Short beep
            __delay_ms(500);
            current_state = JIG_STATE_CHECK_STACK;
            break;
            
        case JIG_STATE_ADVANCE_REJECT:
            // Advance to reject bin
            Jig_SetLEDs(false, true, false); // Red LED
            Jig_Buzz(200); // Longer beep
            __delay_ms(500);
            current_state = JIG_STATE_CHECK_STACK;
            break;
            
        case JIG_STATE_ERROR:
            // Error state - flash red LED
            Jig_SetLEDs(false, true, false);
            __delay_ms(500);
            Jig_SetLEDs(false, false, false);
            __delay_ms(500);
            break;
            
        default:
            current_state = JIG_STATE_IDLE;
            break;
    }
}
