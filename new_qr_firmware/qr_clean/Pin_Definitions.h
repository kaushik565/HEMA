#include<p18cxxx.h>

#define BOARD_VER2 1

#ifdef BOARD_VER2
	#define	RJT_SNS_P	TRISEbits.TRISE2
	#define	RJT_SNS	PORTEbits.RE2
#else
	#error "This firmware targets BOARD_VER2 only"
#endif

// User switches
#define SW_1_P TRISBbits.TRISB2 
#define SW_2_P TRISBbits.TRISB3
#define SW_3_P TRISBbits.TRISB4

#define SW_1 PORTBbits.RB2
#define SW_2 PORTBbits.RB3
#define SW_3 PORTBbits.RB4

// Mechanism control
#define CAT_FB_P TRISEbits.TRISE0
#define CAT_FB LATEbits.LATE0
#define REJECT_SV_P TRISAbits.TRISA4
#define REJECT_SV LATAbits.LATA4
#define ELECT_SOL_P TRISAbits.TRISA3
#define ELECT_SOL LATAbits.LATA3

// Mech sensors
#define BW_SNS_P TRISCbits.TRISC0
#define BW_SNS PORTCbits.RC0
#define FW_SNS_P TRISCbits.TRISC1
#define FW_SNS PORTCbits.RC1
#define MECH_UP_SNS_P TRISCbits.TRISC2
#define MECH_UP_SNS PORTCbits.RC2
#define STACK_SNS PORTCbits.RC4
#define CAT_SNS PORTCbits.RC5

// Tower and aux IO
#define BUZZER_P TRISDbits.TRISD5
#define BUZZER LATDbits.LATD5
#define KILL_P TRISDbits.TRISD4
#define KILL LATDbits.LATD4

// Step clock (if used)
#define	P_ENB_2	TRISDbits.TRISD6
#define	P_CLK_2	TRISDbits.TRISD7
#define	ENB_2	LATDbits.LATD6
#define	CLK_2	LATDbits.LATD7

// UART to Raspberry Pi
#define RX_PIC_P TRISCbits.TRISC7
#define TX_PIC_P TRISCbits.TRISC6
#define RX_PIC LATCbits.LATC7
#define TX_PIC LATCbits.LATC6

// Handshake lines with Pi
#define INT_PIC_P TRISBbits.TRISB5
#define RASP_IN_PIC_P TRISBbits.TRISB6
#define SHD_PIC_P TRISBbits.TRISB7
#define INT_PIC LATBbits.LATB5    
#define RASP_IN_PIC PORTBbits.RB6
#define SHD_PIC PORTBbits.RB7
