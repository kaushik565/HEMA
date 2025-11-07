#include "SBC_Rpi.h"

extern unsigned int TM;
extern unsigned char BF_Count,BF_Count_Limit;
unsigned char sbc_ready=0, sbc_disabled, qr_disabled;

void SBC_UARTInit(void){
	char Test1 = 0x24; // BRGH=1
	TXSTA = Test1; RCSTA = 0x90; BAUDCON = 0x08; SPBRGH = 0x00; SPBRG = 0x68; // ~9600 @ 48MHz
}

void disable_sbc(void){ display(1,0,"OFF DATA?",0); do{ if(!SW_3 && !SW_2){ sbc_disabled=1; write_eeprom( sbc_disabled,0); display(1,0,"OFF",2); break; } } while(SW_1); }
void enable_sbc(void){ display(1,0,"ON DATA?",0);  do{ if(!SW_3 && !SW_2){ sbc_disabled=0; write_eeprom( sbc_disabled,0); display(1,0,"ON",2); break; } } while(SW_1); }
void disable_qr(void){ display(1,0,"OFF QR?",0); do{ if(!SW_3 && !SW_2){ qr_disabled=1; write_eeprom( qr_disabled,2); display(1,0,"OFF",2); break; } } while(SW_1); }
void enable_qr(void){ display(1,0,"ON QR?",0);  do{ if(!SW_3 && !SW_2){ qr_disabled=0; write_eeprom( qr_disabled,2); display(1,0,"ON",2); break; } } while(SW_1); }

char wait_busy_rpi(void){ unsigned int count=500; if(RASP_IN_PIC==1){ while(RASP_IN_PIC==1){ if(!count){ return 1; } if(!SW_3 && !SW_2){ return 0; } DELAY_10mS(); count--; } } return 0; }

void wait_ready_rpi(void){ unsigned int count=26000; if(RASP_IN_PIC==0){ display(1,"INITIALIZING",0,0); while(RASP_IN_PIC==0){ if(!count){ display(0,0,"SBC Er-1",0); while(1); } if(!SW_3 && !SW_2){ return; } DELAY_10mS(); count--; } } sbc_ready=1; }

unsigned char wait_log_rpi(void){ char in; unsigned int count=3000; while(SW_1 || SW_2){ if (PIR1bits.RCIF == 1){ PIR1bits.RCIF = 0; in = RCREG; if(in=='S')return 0; DELAY_1mS(); if(!(count--)){ return 1; } } } }

void flush_uart(void){ char temp; RCSTAbits.CREN=0; PIR1bits.RCIF =0; RCSTAbits.CREN=1; temp=RCREG; temp=RCREG; }

const char* READER_ERROR="READER ERROR"; const char* NO_QR="NO QR"; const char* QR_FB_ERROR="QR FB ERROR"; const char* QR_TOUT="QR T.OUT";

char wait_for_qr(void){ unsigned int tcount=12000; char in=0; PIR1bits.RCIF = 0; display(0,"READING QR",0,0); while(SW_3 || SW_2){ if (PIR1bits.RCIF == 1){ PIR1bits.RCIF = 0; in = RCREG; switch (in){ case 'A': { cont.qr_error=0; return 0; } case 'R': { DELAY_100mS(); cont.qr_error++; BUZZER=1; DELAY_100mS(); display(1,0,"REJECTED",2); BUZZER=0; return 1; } case 'S': { DELAY_100mS(); cont.qr_error++; BUZZER=1; DELAY_100mS(); display(1,0,"SCANNER ERROR",2); BUZZER=0; return 3; } case 'H': { DELAY_100mS(); cont.qr_error++; display(1,"SCANNER HW ERROR",press_sm_string,1); while (SW_3&&SW_2){ BUZZER=~BUZZER; DELAY_500mS(); } BUZZER=0; return 1; } case 'N': { DELAY_100mS(); cont.qr_error++; display(1,"LENGTH ERROR.",press_sm_string,1); while (SW_3&&SW_2){ BUZZER=~BUZZER; DELAY_500mS(); } BUZZER=0; return 1; } case 'C': { DELAY_100mS(); BUZZER=1; display(1,"REPEATED TESTING.",press_sm_string,1); while (SW_3&&SW_2){ BUZZER=~BUZZER; DELAY_500mS(); } BUZZER=0; LCD_Cmd(LCD_CLEAR); return 2; } case 'D': { DELAY_100mS(); BUZZER=1; display(1,"DUPLICATE QR.",press_sm_string,1); while (SW_3&&SW_2){ BUZZER=~BUZZER; DELAY_500mS(); } BUZZER=0; LCD_Cmd(LCD_CLEAR); return 2; } case 'Q': { DELAY_100mS(); cont.qr_error++; BUZZER=1; display(1,0,NO_QR,1); BUZZER=0; return 3; } case 'L': { DELAY_100mS(); cont.qr_error++; display(1,"LENGTH ERROR.",press_sm_string,1); while (SW_3&&SW_2){ BUZZER=~BUZZER; DELAY_500mS(); } BUZZER=0; LCD_Cmd(LCD_CLEAR); return 1; } case 'B': { DELAY_100mS(); cont.qr_error++; display(1,"LOGGING ERROR.",press_sm_string,1); while (SW_3&&SW_2){ BUZZER=~BUZZER; DELAY_500mS(); } BUZZER=0; LCD_Cmd(LCD_CLEAR); return 1; } default: { DELAY_100mS(); cont.qr_error++; BUZZER=1; display(1,0,QR_FB_ERROR,1); BUZZER=0; return 3; } } } if(!tcount){ DELAY_100mS(); cont.qr_error++; BUZZER=1; display(1,0,QR_TOUT,1); BUZZER=0; return 3; } DELAY_10mS(); tcount--; } }

void sent_rpi_uint16(unsigned int data){ while (!TXSTAbits.TRMT); TXREG = ((int) data/1000+'0'); while (!TXSTAbits.TRMT); TXREG = (((data)%1000)/100+'0'); while (!TXSTAbits.TRMT); TXREG = (((data)%100)/10+'0'); while (!TXSTAbits.TRMT); TXREG = ((data)%10+'0'); }

void Init_UART_RPi(void) { RX_PIC_P=1; TX_PIC_P=0; TXSTA = 0x24; RCSTA = 0x90; BAUDCON = 0x08; SPBRGH = 0x00; SPBRG = 0x68; IPR1bits.RCIP = 1; INTCON = 0x00; PIE1bits.RCIE = 0; }

void read_string_rpi(char* string_in){ PIR1bits.RCIF = 0; while(1){ while(PIR1bits.RCIF == 0); (*string_in)=RCREG; if((*string_in)==0)return; string_in++; } }
void write_ram_string_rpi(ram char* in) { while(*in){ while (!TXSTAbits.TRMT); TXREG = *in; in++; } }
void write_ram_rpi(ram  char data) { while (!TXSTAbits.TRMT); TXREG = data; }
void write_rom_string_rpi(rom  char *in) { while(*in){ while (!TXSTAbits.TRMT); TXREG = (*in); in++; } }
void write_rpi_sect_brk(void) { rom  char *in="-,-\n"; while(*in){ while (!TXSTAbits.TRMT); TXREG = (*in); in++; } }
void write_rom_rpi(const char data) { while (!TXSTAbits.TRMT); TXREG = data; }
void write_rom_rpi_nl(void) { while (!TXSTAbits.TRMT); TXREG = '\n'; }
