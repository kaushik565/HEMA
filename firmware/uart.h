/**
 * UART Communication Module for HEMA Jig Firmware
 * PIC18F4550 Microcontroller
 * 
 * Handles serial communication with Raspberry Pi
 */

#ifndef UART_H
#define UART_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

// Function prototypes
void UART_Init(void);
void UART_Write(uint8_t data);
bool UART_DataAvailable(void);
uint8_t UART_Read(void);
bool UART_ReadWithTimeout(uint8_t *data, uint16_t timeout_ms);
void UART_WriteString(const char *str);
void UART_Flush(void);

#endif // UART_H
