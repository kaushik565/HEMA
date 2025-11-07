#include <p18cxxx.h>
#include <delays.h>
#include "SBC_Rpi.h"
#include "Functions.h"
#include "service.h"
#include "Pin_Definitions.h"

#pragma config CP0=ON, CP1=ON, CP2=ON, CP3=ON
#pragma config CPB=OFF, WRT0=OFF, WRT1=OFF, WRTB=OFF, WRTC=OFF
#pragma config EBTR0=OFF, EBTR1=OFF, EBTRB=OFF

#pragma romdata dataEEPROM=0xF00000
rom unsigned char FirstByte[3]={0,20,0};
#pragma romdata

unsigned long count=0, pass_count=0;
unsigned int TM, BaseValue, C_BaseValue;
const char* press_sm_string = "PRESS S.M";
static unsigned char reject_flag = 0;

void YourHighPriorityISRCode(void);
void YourLowPriorityISRCode(void);
void port_init(void);
void catFB_forward(void);
void mechUp_catFB_Back(void);
void check_stack(void);
void reset_mechanism(void);
void reject_on(void);
void reject_off(void);

#pragma code high_vector=0x08
void interrupt_at_high_vector(void){ _asm GOTO YourHighPriorityISRCode _endasm }
#pragma code
#pragma code low_vector=0x18
void interrupt_at_low_vector(void){ _asm GOTO YourLowPriorityISRCode _endasm }
#pragma code
#pragma interrupt YourHighPriorityISRCode
void YourHighPriorityISRCode(){ if (INTCON3bits.INT2IF) { INTCON3bits.INT2IF=0; } }
#pragma interruptlow YourLowPriorityISRCode
void YourLowPriorityISRCode(){ }
#pragma code

#define STACK_END_SKIP 0
#define PLATE_STUCK_RETRY 5

void main(void){
	char qr_result = 0;
	port_init(); I2C_Init1(); ADC_Init(); LCD_Begin(0b01001110);
#ifdef BOARD_VER2
	display(1,"CARTRIDGE QR","SCANNER JIG v2.3",1);
#else
	display(1,"CARTRIDGE QR","SCANNER JIG",1);
#endif
	TOWER_OFF; sbc_disabled=read_eeprom(0); DELAY_250mS(); qr_disabled=read_eeprom(2);
	Init_PowerInt(); SBC_UARTInit(); INTCONbits.TMR0IE = 1; INTCONbits.TMR0IF = 0;
	if(sbc_disabled==1){ sbc_ready=0; } else { wait_ready_rpi(); }
	display(1,"PRESS START",0,0);
	while(SW_3){ if(!SW_2){ service_menu(); } }
	while(!SW_3);
	while(1){
		check_stack(); display_counts(); ELECT_SOL = 0; DELAY_100mS();
		catFB_forward(); ELECT_SOL = 1; DELAY_250mS();
		flush_uart(); qr_result = (sbc_ready==1)? wait_for_qr():3;
		if(qr_result == 0){ reject_flag = 0; reject_off(); pass_count++; } else { reject_flag = 1; reject_on(); }
		count++;
		DELAY_500mS(); ELECT_SOL = 0; DELAY_100mS(); mechUp_catFB_Back(); if(sbc_ready==1){ write_rom_rpi(0); }
	}
}

void reject_on(void){ unsigned int i=0; REJECT_SV=1; #ifdef BOARD_VER2 while(RJT_SNS){ i++; DELAY_1mS(); if(i==6000){ POWER_INT_ENABLE; display(1,0,"REJECT PLT STUCK",2); while(1); } } #endif }
void reject_off(void){ unsigned int i=0; REJECT_SV=0; #ifdef BOARD_VER2 while(!RJT_SNS){ i++; DELAY_1mS(); if(i==6000){ POWER_INT_ENABLE; display(1,0,"PASS PLT STUCK",2); while(1); } } #endif }

void catFB_forward(void){ unsigned int i=0,plate_stuck_retry=PLATE_STUCK_RETRY; CAT_FB=1; while(!FW_SNS){ i++; DELAY_1mS(); if(i==5000){ if(plate_stuck_retry--){ CAT_FB=0; i=0; while(!BW_SNS){ i++; DELAY_1mS(); if(i==6000){ display(1,0,"CAT PLT BK STUCK",0); POWER_INT_ENABLE; ELECT_SOL=0; while(1); } } i=0; ELECT_SOL=1; CAT_FB=1; continue; } else { display(1,0,"CAT PLT FW STUCK",0); POWER_INT_ENABLE; ELECT_SOL=0; reset_mechanism(); mech_error_botton(); DELAY_100mS(); LCD_Begin(0b01001110); display_counts(); i=0; plate_stuck_retry=PLATE_STUCK_RETRY; ELECT_SOL=1; CAT_FB=1; continue; } } } }
void mechUp_catFB_Back(void){ unsigned int i=0; CAT_FB=0; i=0; while(!BW_SNS){ i++; DELAY_1mS(); if(i==10000){ POWER_INT_ENABLE; display(1,0,"CAT PLT BK STUCK",5); while(1); } } }

void check_stack(void){ static unsigned char stack_skip=STACK_END_SKIP; if(STACK_SNS==0){ if(!stack_skip){ POWER_INT_ENABLE; reset_mechanism(); TOWER_ON; DisplayStackEmpty(); BUZZER=0; TOWER_OFF; while(!SW_3 ); stack_skip=STACK_END_SKIP; LCD_Begin(0b01001110); } else { stack_skip--; } } else { BUZZER=0; stack_skip=STACK_END_SKIP; } }

void reset_mechanism(void){ BUZZER=0; CAT_FB=0; REJECT_SV=0; ELECT_SOL=0; TOWER_OFF; }

void port_init(void){
	BUZZER_P=0; BUZZER=0; RJT_SNS_P=1;
	INTCON2bits.RBPU=0; /* enable PORTB pull-ups */
	P_ENB_2=0; P_CLK_2=0; /* stepper lines as outputs (if used) */
	/* Configure analog pins as inputs where needed */
	TRISAbits.TRISA0=1; TRISAbits.TRISA1=1; TRISAbits.TRISA2=1; TRISAbits.TRISA3=1;
	SW_1_P=1; SW_2_P=1; SW_3_P=1;
	INT_PIC_P=0; RASP_IN_PIC_P=1; SHD_PIC_P=1; INT_PIC=0;
	CAT_FB_P=0; REJECT_SV_P=0; ELECT_SOL_P =0; CAT_FB=0; REJECT_SV=0; ELECT_SOL =0;
	BW_SNS_P=1; FW_SNS_P=1; /* sensors as inputs */
	/* Disable USB so RC4/RC5/RC6/RC7 function as GPIO/UART */
	UCONbits.USBEN = 0; UCFGbits.UTRDIS = 1;
}
