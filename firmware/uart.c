/**
 * UART Communication Module Implementation
 * PIC18F4550 Microcontroller @ 48MHz
 * 
 * Serial communication with Raspberry Pi at 115200 baud
 */

#include "uart.h"
#include "config.h"
#include <xc.h>

/**
 * Initialize UART module
 * Baud rate: 115200 @ 48MHz
 * Format: 8N1 (8 data bits, no parity, 1 stop bit)
 */
void UART_Init(void) {
    // UART Configuration for 115200 baud @ 48MHz
    // SPBRG = (Fosc / (16 * Baud)) - 1
    // SPBRG = (48000000 / (16 * 115200)) - 1 = 25.04 ≈ 25 (0x19)
    // Actual baud = 48000000 / (16 * (25 + 1)) = 115384 (0.16% error)
    
    // For better accuracy, use BRGH=1 (high speed mode)
    // SPBRG = (Fosc / (4 * Baud)) - 1
    // SPBRG = (48000000 / (4 * 115200)) - 1 = 103.17 ≈ 103 (0x67)
    // Actual baud = 48000000 / (4 * (103 + 1)) = 115384 (0.16% error)
    
    // Even better: use BRG16=1 for 16-bit baud rate generator
    // SPBRG = (Fosc / (4 * Baud)) - 1 (with BRG16=1, BRGH=1)
    // SPBRG = (48000000 / (4 * 115200)) - 1 = 103.17 ≈ 103 (0x0067)
    
    TXSTA = 0x24;   // TX enable, BRGH=1 (high speed), 8-bit, async
    RCSTA = 0x90;   // Serial port enable, continuous receive, 8-bit
    BAUDCON = 0x08; // BRG16=1 for 16-bit baud rate generator
    
    // Set baud rate (115200 @ 48MHz)
    SPBRGH = 0x00;  // High byte
    SPBRG = 0x67;   // Low byte (103 decimal = 0x67)
    
    // Clear any receive errors
    if (RCSTAbits.OERR) {
        RCSTAbits.CREN = 0; // Clear overrun by toggling CREN
        RCSTAbits.CREN = 1;
    }
    if (RCSTAbits.FERR) {
        uint8_t dummy = RCREG; // Clear framing error by reading
        (void)dummy;
    }
}

/**
 * Write a byte to UART
 */
void UART_Write(uint8_t data) {
    while (!TXSTAbits.TRMT); // Wait until transmit shift register is empty
    TXREG = data;            // Write data to transmit register
}

/**
 * Check if data is available to read
 */
bool UART_DataAvailable(void) {
    return PIR1bits.RCIF; // Check receive interrupt flag
}

/**
 * Read a byte from UART (blocking)
 */
uint8_t UART_Read(void) {
    // Check for errors
    if (RCSTAbits.OERR) {
        RCSTAbits.CREN = 0; // Clear overrun error
        RCSTAbits.CREN = 1;
    }
    if (RCSTAbits.FERR) {
        uint8_t dummy = RCREG; // Clear framing error
        (void)dummy;
    }
    
    while (!PIR1bits.RCIF); // Wait for data
    return RCREG;           // Return received data
}

/**
 * Read a byte with timeout
 * Returns true if data received, false on timeout
 */
bool UART_ReadWithTimeout(uint8_t *data, uint16_t timeout_ms) {
    uint16_t count = 0;
    
    // Check for errors first
    if (RCSTAbits.OERR) {
        RCSTAbits.CREN = 0;
        RCSTAbits.CREN = 1;
    }
    if (RCSTAbits.FERR) {
        uint8_t dummy = RCREG;
        (void)dummy;
    }
    
    // Wait for data or timeout
    while (!PIR1bits.RCIF && count < timeout_ms) {
        __delay_ms(1);
        count++;
    }
    
    if (PIR1bits.RCIF) {
        *data = RCREG;
        return true;
    }
    
    return false; // Timeout
}

/**
 * Write a null-terminated string to UART
 */
void UART_WriteString(const char *str) {
    while (*str) {
        UART_Write(*str++);
    }
}

/**
 * Flush receive buffer
 */
void UART_Flush(void) {
    uint8_t dummy;
    while (PIR1bits.RCIF) {
        dummy = RCREG;
    }
    (void)dummy;
    
    // Clear any errors
    if (RCSTAbits.OERR) {
        RCSTAbits.CREN = 0;
        RCSTAbits.CREN = 1;
    }
}
