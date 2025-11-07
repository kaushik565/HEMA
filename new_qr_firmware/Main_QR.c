


#include <p18cxxx.h>
#include <delays.h>
#include "SBC_Rpi.h"
#include "Functions.h"
#include "service.h"
#include "Pin_Definitions.h"


        #pragma config CP0      = ON
        #pragma config CP1      = ON
    	#pragma config CP2      = ON
	    #pragma config CP3      = ON
        #pragma config CPB      = OFF
        #pragma config WRT0     = OFF
        #pragma config WRT1     = OFF
        #pragma config WRTB     = OFF       
        #pragma config WRTC     = OFF
        #pragma config EBTR0    = OFF
        #pragma config EBTR1    = OFF
        #pragma config EBTRB    = OFF


#pragma romdata dataEEPROM=0xF00000
rom unsigned char FirstByte[3]={0,20,0};	
#pragma romdata


unsigned long count=0, pass_count=0;
unsigned int TM, BaseValue, C_BaseValue;
const char* press_sm_string = "PRESS S.M";
static unsigned char testing = 0;
static unsigned char reject_flag = 0;


void YourHighPriorityISRCode(void);
void YourLowPriorityISRCode(void);
void main(void);
void port_init(void);
void catFB_forward(void);
void mechUp_catFB_Back(void);
void check_stack(void);
void reset_mechanism(void);
void reject_on(void);
void reject_off(void);



#pragma code high_vector=0x08
void interrupt_at_high_vector(void)
{
	_asm GOTO YourHighPriorityISRCode _endasm
}
#pragma code

#pragma code low_vector=0x18
void interrupt_at_low_vector(void)
{
	_asm GOTO YourLowPriorityISRCode _endasm
}
#pragma code
#pragma interrupt YourHighPriorityISRCode
void YourHighPriorityISRCode()
{
	    if (INTCON3bits.INT2IF) {
		    char temp=0;
	        DELAY_50mS();
	        if(!(SW_1)) {
	            DELAY_50mS();
	            if(!(SW_1)) {
	                DELAY_100mS();
	                        if(!(SW_1)) {
		                        reset_mechanism();
	                            INTCONbits.GIE = 0;
	                            ENB_2=1;
	                            
	                            ValveM1_R=0;
								ValveM2_R=0;
								ValveM1_F=0;
								ValveM2_F=0;
								
								if(testing){
									
									Reset();
								}
	                            DELAY_50mS();
	                            write_rom_rpi(0);
							    LCD_Begin(0b01001110);
							   
							    LCD_Cmd(LCD_CLEAR);
								LCD_Cmd(LCD_FIRST_ROW );
								LCD_Print("TURNING OFF");
								
	                            DELAY_250mS();
	                            RASP_IN_PIC_P=1;
	                            while(RASP_IN_PIC==0) {
	                                temp++;
	                                DELAY_1S();
	                                if(temp==40){
		                                
		                                break;
		                             } 
	                            }
	                            DELAY_1S();
	                            write_rom_rpi(0);
	                            INT_PIC_P=0;
	                            INT_PIC=0;
	                            SHD_PIC_P=1;
	                            temp=0;
	                            while (SHD_PIC) {
	                                DELAY_1S();
	                                temp++;
	                                INT_PIC=~INT_PIC;
	                                if(temp==100){
		                                
		                                KILL=0;
		                            }  
	                            }
	                            KILL=0;
	                }
	            }
	        }
	        INTCON3bits.INT2IF=0;
	    }
	    	
}	

#pragma interruptlow YourLowPriorityISRCode
void YourLowPriorityISRCode()
{
	
	
	
	

}	

#pragma code



#define STACK_END_SKIP 0
#define PLATE_STUCK_RETRY 5








void main(void)
{
	char qr_result = 0;
	port_init();
	I2C_Init1();
	ADC_Init();
	LCD_Begin(0b01001110);
#ifdef BOARD_VER2
	display(1,"CARTRIDGE QR","SCANNER JIG v2.3",1);
#else
	display(1,"CARTRIDGE QR","SCANNER JIG v1.3",1);
#endif
	TOWER_OFF;
	sbc_disabled=read_eeprom(0);
	DELAY_250mS();
	qr_disabled=read_eeprom(2);
	Init_PowerInt();
	SBC_UARTInit();
	INTCONbits.TMR0IE = 1;
	INTCONbits.TMR0IF = 0;
	if(RCONbits.RI){
		
	}
	if(sbc_disabled==1){
		sbc_ready=0;
	} else {
		wait_ready_rpi();
	}
	display(1,"PRESS START",0,0);
	while(SW_3){
		if(!SW_2){
			service_menu();
		}
	}
	while(!SW_3);
	
	while(1){
		check_stack();
		display_counts();

		
		ELECT_SOL = 0; 
		DELAY_100mS();

		
		catFB_forward();

		
		ELECT_SOL = 1; 
		DELAY_250mS();

		
		
		
		flush_uart();
		if(sbc_ready==1){
			qr_result = wait_for_qr(); 
		} else {
			qr_result = 3; 
		}

		
		
		if(qr_result == 0){
			reject_flag = 0; 
			reject_off();
		} else {
			reject_flag = 1; 
			reject_on();
		}
		DELAY_500mS();

		
		ELECT_SOL = 0;
		DELAY_100mS();

		
		mechUp_catFB_Back();
		
		
		if(sbc_ready==1){
			write_rom_rpi(0); 
		}
	}
}

void reject_on(void){
	unsigned int i=0;
	REJECT_SV=1; 
	#ifdef BOARD_VER2
	while(RJT_SNS){
		i++;
		DELAY_1mS();
		if(i==6000){
			POWER_INT_ENABLE;
			display(1,0,"REJECT PLT STUCK",2);
			mech_error_loop();
		}
	}
	#endif
}

void reject_off(void){
	unsigned int i=0;
	REJECT_SV=0; 
	#ifdef BOARD_VER2
	while(!RJT_SNS){
		i++;
		DELAY_1mS();
		if(i==6000){
			POWER_INT_ENABLE;
			display(1,0,"PASS PLT STUCK",2);
			mech_error_loop();
		}
	}
	#endif
}



void catFB_forward(void){
	unsigned int i=0,plate_stuck_retry=PLATE_STUCK_RETRY;
	CAT_FB=1; 
		while(!FW_SNS){
			i++;
			DELAY_1mS();
			if(i==5000){
				if(plate_stuck_retry--){
					CAT_FB=0;
					i=0;
					while(!BW_SNS){
						i++;
						DELAY_1mS();
						if(i==6000){
							display(1,0,"CAT PLT BK STUCK",0);
							POWER_INT_ENABLE;
							ELECT_SOL=0; 
							mech_error_loop();
						
						}
					}
				
					
					i=0;
					ELECT_SOL=1; 
					CAT_FB=1; 
					continue;
				}
				else {
					display(1,0,"CAT PLT FW STUCK",0);
					POWER_INT_ENABLE;
					ELECT_SOL=0; 
					reset_mechanism();
					mech_error_botton();
					DELAY_100mS();
					LCD_Begin(0b01001110);
					display_counts();
					
					i=0;
					plate_stuck_retry=PLATE_STUCK_RETRY;
					ELECT_SOL=1; 
					CAT_FB=1; 
					continue;
				}
			}	
		}
}
void mechUp_catFB_Back(void){
	unsigned int i=0;
	CAT_FB=0;
	i=0;
	while(!BW_SNS){
		i++;
		DELAY_1mS();
		if(i==10000){
			
			POWER_INT_ENABLE;
			display(1,0,"CAT PLT BK STUCK",5);
			mech_error_loop();
			
		}
	}			
}

void check_stack(void){
	 static unsigned char stack_skip=STACK_END_SKIP;
	if(STACK_SNS==0){
			
			if(!stack_skip){
				POWER_INT_ENABLE;
				reset_mechanism();
				
			
				TOWER_ON;	
				DisplayStackEmpty();
				
				BUZZER=0;
				TOWER_OFF;
				while(!SW_3 );	
				stack_skip=STACK_END_SKIP;
				LCD_Begin(0b01001110);				
			}
			else {
				stack_skip--;
			}	
		}else {
			
			BUZZER=0;
			stack_skip=STACK_END_SKIP;
		}
}

void check_key_intrpt(void){

}

void reset_mechanism(void){

	
	BUZZER=0;
	CAT_FB=0;
	REJECT_SV=0;
	ELECT_SOL=0;
	TOWER_OFF;
}

const char* PASSs="PASS";
const char* FAILs="FAIL";
char cat_test(void){
	char qr_result=0, retry=3;
	
	count++;
	while(retry--){	
		if(sbc_ready==1){
			if(!qr_disabled){
				if(retry)
					write_rom_rpi(20);
				else 
					write_rom_rpi(19);
				if(wait_busy_rpi()){
					if(retry)
						write_rom_rpi(20);
					else 
						write_rom_rpi(19);
					if(wait_busy_rpi()){
						if(retry)
							write_rom_rpi(20);
						else 
							write_rom_rpi(19);
						if(wait_busy_rpi()){
							display(0,0,"SBC Er-2",0);
							mech_error_loop();
						}
					}
				}
				
				flush_uart();
				qr_result=wait_for_qr();
				if(qr_result==0){
					display(1,0,PASSs,0);
		
					pass_count++;
					display_counts();
					return 0; 	
				}
				else if(qr_result==1){
					return 1;
				}
				else if(qr_result==2){
					return reject_flag;
				}
				else if(qr_result==3){
					
					display(0,"RETRYING",0,0);
					DELAY_500mS();
				}	
				
				else {
					return 1;
				}
			}	
		}
	}
	
	display(1,"QR NOT READABLE ",press_sm_string,1);
	
	while (SW_3&&SW_2){
		BUZZER=~BUZZER;
		DELAY_500mS();
	}

	BUZZER=0;

	return 1;
}

void port_init(void){

	BUZZER_P=0;
	BUZZER=0;
	RJT_SNS_P=1;
	PORTEbits.RDPU=0;
	
	P_ENB_2=0;
	P_CLK_2=0;

	TRISAbits.TRISA0=1;
	TRISAbits.TRISA1=1;
	TRISAbits.TRISA2=1;	
	TRISAbits.TRISA3=1;
	SW_1_P=1;
	SW_2_P=1;
	SW_3_P=1;
	
	LM_SW_DET_PORT=1;
	INT_PIC_P=0;
	RASP_IN_PIC_P=1;
	SHD_PIC_P=1;
	INT_PIC=0;
	
	CAT_FB_P=0;
	REJECT_SV_P=0;
	ELECT_SOL_P =0;
	
	CAT_FB=0;
	REJECT_SV=0;
	ELECT_SOL =0;
	
	BW_SNS_P=1;
	FW_SNS_P=1;

	
	UCONbits.USBEN = 0; 
	UCFGbits.UTRDIS = 1; 

}










