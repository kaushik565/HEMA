/**
 * Jig Mechanism Control Module for HEMA Firmware
 * PIC18F4550 Microcontroller
 * 
 * Controls pneumatic cylinders and mechanisms for cartridge handling
 */

#ifndef JIG_CONTROL_H
#define JIG_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

// Jig state machine states
typedef enum {
    JIG_STATE_IDLE,
    JIG_STATE_WAIT_SAFETY,
    JIG_STATE_CHECK_STACK,
    JIG_STATE_PUSH_EXTEND,
    JIG_STATE_PUSH_RETRACT,
    JIG_STATE_WAIT_DETECTION,
    JIG_STATE_WAIT_SCAN,
    JIG_STATE_ADVANCE_GOOD,
    JIG_STATE_ADVANCE_REJECT,
    JIG_STATE_ERROR
} JigState_t;

// Function prototypes
void Jig_Init(void);
void Jig_Start(void);
void Jig_Stop(void);
void Jig_ProcessStateMachine(void);
bool Jig_IsSafetyOK(void);
bool Jig_IsStackPresent(void);
bool Jig_IsCartridgeAtScanner(void);
void Jig_ExtendPusher(void);
void Jig_RetractPusher(void);
void Jig_StopPusher(void);
void Jig_RaiseStoropper(void);
void Jig_LowerStopper(void);
void Jig_StopStopper(void);
void Jig_SetLEDs(bool green, bool red, bool yellow);
void Jig_Buzz(uint16_t duration_ms);
void Jig_EmergencyStop(void);

#endif // JIG_CONTROL_H
