/********************************************************************
 FileName:		Main_PCR.c
 Dependencies:	See INCLUDES section
 Processor:		PIC18F4550 Microcontroller
 Hardware:		ACTJv20(RJSR) Cartridge QR Validation Jig
 Complier:  	Microchip C18 (for PIC18)
 ********************************************************************
 File Description:
 	Main control loop for cartridge QR scanning and pass/reject jig.
 	Communicates with Raspberry Pi via UART for QR validation.
 	
 	Operation:
 	- Push cartridge to scan position
 	- Raise stopper to hold cartridge
 	- Send scan request to Pi (byte 20 = 0x14)
 	- Wait for Pi response ('A'=pass, 'R'=reject)
 	- Actuate rejection mechanism based on result
 	- Return pusher to home position
 	- Repeat for next cartridge
 *******************************************************************/

/** INCLUDES *******************************************************/
#include <p18cxxx.h>
#include <delays.h>
#include "SBC_Rpi.h"
#include "Functions.h"
#include "service.h"
#include "Pin_Definitions.h"

/** CONFIGURATION **************************************************/
        #pragma config CP0      = ON
        #pragma config CP1      = ON
    	#pragma config CP2      = ON
	    #pragma config CP3      = ON
        #pragma config CPB      = OFF
        #pragma config WRT0     = OFF
        #pragma config WRT1     = OFF
        #pragma config WRTB     = OFF       // Boot Block Write Protection
        #pragma config WRTC     = OFF
        #pragma config EBTR0    = OFF
        #pragma config EBTR1    = OFF
        #pragma config EBTRB    = OFF


#pragma romdata dataEEPROM=0xF00000
rom unsigned char FirstByte[3]={0,20,0};	//EEPROM
#pragma romdata

/** GLOBAL VARIABLES ***********************************************/
unsigned long count=0, pass_count=0;

/** FUNCTION PROTOTYPES ********************************************/
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

/** INTERRUPT SERVICE ROUTINES *************************************/
/** INTERRUPT SERVICE ROUTINES *************************************/
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
	                            // EJ_MOTOR=0;	
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
	    /*if(INTCONbits.TMR0IF){
			INTCONbits.TMR0IF=0;
			BUZZER=~BUZZER;
			if(STACK_SNS==1){
			T0CONbits.TMR0ON = 0;
			BUZZER=0;
		}
	}*/	
}	//This return will be a "retfie fast", since this is in a #pragma interrupt section 

#pragma interruptlow YourLowPriorityISRCode
void YourLowPriorityISRCode()
{
	//Check which interrupt flag caused the interrupt.
	//Service the interrupt
	//Clear the interrupt flag
	//Etc.

}	//This return will be a "retfie", since this is in a #pragma interruptlow section

#pragma code

/** DECLARATIONS ***************************************************/

#define STACK_END_SKIP 0
#define PLATE_STUCK_RETRY 5
// --- Removed unused error logic and vacuum/valve code ---




/******************************************************************************
 * Function:        void main(void)
 * PreCondition:    None
 * Side Effects:    None
 * Overview:        Main program entry point.
 * Note:            None
 *******************************************************************/

// Only main entry for PIC18
void main(void)
{
	// Only keep essential hardware init and main loop
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
		// post(70);
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
	// --- Main cartridge scanning loop ---
	// Uses existing firmware functions: wait_for_qr(), write_rom_rpi()
	// Protocol: Send scan request (20=retry, 19=final), wait for 'A'/'R' response
	char qr_result = 0;
	char reject_flag = 0;
	
	while(1){
		check_stack();
		display_counts();

		// Lower stopper to release previous cartridge (if any)
		ELECT_SOL = 0; // DOWN: release
		DELAY_100mS();

		// Move pusher forward: push next cartridge to scan position
		catFB_forward();

		// Raise stopper to hold cartridge at scan position
		ELECT_SOL = 1; // UP: hold
		DELAY_250mS();

		// --- Send scan request to Pi and wait for result ---
		// write_rom_rpi(20) = scan with retry (0x14)
		// write_rom_rpi(19) = final scan attempt (0x13)
		flush_uart();
		if(sbc_ready==1){
			write_rom_rpi(20); // Send scan request (binary byte 0x14)
			qr_result = wait_for_qr(); // Wait for 'A'/'R'/'S' etc response
		} else {
			qr_result = 3; // SBC not ready, treat as error
		}

		// --- Act on QR scan result ---
		// wait_for_qr() returns: 0=PASS, 1=REJECT, 2=DUPLICATE, 3=ERROR
		if(qr_result == 0){
			reject_flag = 0; // PASS: do not reject
			reject_off();
		} else {
			reject_flag = 1; // REJECT/DUPLICATE/ERROR: reject
			reject_on();
		}
		DELAY_500mS();

		// Lower stopper to release cartridge
		ELECT_SOL = 0;
		DELAY_100mS();

		// Move pusher back to home
		mechUp_catFB_Back();
		
		// Stop recording if SBC is ready
		if(sbc_ready==1){
			write_rom_rpi(0); // Stop recording command
		}
	}
}

void reject_on(void){
	unsigned int i=0;
	REJECT_SV=1; //plate down
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
	CAT_FB=1; //cartride forward
		while(!FW_SNS){
			i++;
			DELAY_1mS();
			if(i==5000){
				if(plate_stuck_retry--){
					CAT_FB=0;//Move cartridge plate backward
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
					ELECT_SOL=1; //solenoid stopper down
					CAT_FB=1; //cartride forward
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
					ELECT_SOL=1; //solenoid stopper down
					CAT_FB=1; //cartride forward
					continue;
				}
			}	
		}
}
void mechUp_catFB_Back(void){
	unsigned int i=0;
	CAT_FB=0;//Move cartridge plate backward
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
			//if(!T0CONbits.TMR0ON)T0CONbits.TMR0ON = 1;
			if(!stack_skip){
				POWER_INT_ENABLE;
				reset_mechanism();
				
			//	T0CONbits.TMR0ON = 0;
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
			//T0CONbits.TMR0ON = 0;
			BUZZER=0;
			stack_skip=STACK_END_SKIP;
		}
}

void check_key_intrpt(void){
/*	unsigned int wait_time=400;
	if(INTCON3bits.INT2IF){
		display(1,"SHUTDOWN?","START->PAUSE",0);
		DELAY_500mS();
		POWER_INT_ENABLE;
		while(SW_3){
			wait_time--;
			DELAY_50mS();
			if(!wait_time){	
				POWER_INT_DISABLE;
				return;
			}
		}
	
		while(!SW_3);
		display(1,"SHUTDOWN?","START?",1);
		while(SW_3);
		while(!SW_3);
		POWER_INT_DISABLE;
	}*/
}

void reset_mechanism(void){
//	T0CONbits.TMR0ON = 0;
	//INTCONbits.TMR0IE = 0; 
	BUZZER=0;
	valve_right_idle();	
	CAT_FB=0;//Move cartridge plate backward
	REJECT_SV=0;//Move reject plate forward
	ELECT_SOL=0;//stoper up	
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
				//DELAY_100mS();
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
					//return 1;
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

char ValveTest(void)
{
  
}

void main(void);


void port_init(void){

	BUZZER_P=0;
	BUZZER=0;
	#ifdef BOARD_VER2
		RJT_SNS_P=1;
		VAC_VLV_P=0;
		VAC_VLV=0;
	#else
		P_Valve_1_R=0;
		P_Valve_1_L=0;
		Valve_1_L=0;
		Valve_1_R=1;
	#endif

	P_ValveM1_R=0;
	P_ValveM1_F=0;
	ValveM1_R=0;
	ValveM1_F=0;
	P_ValveM2_R=0;
	P_ValveM2_F=0;
	ValveM2_R=0;
	ValveM2_F=0;
	PORTEbits.RDPU=0;
	//LATE=0x06;
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

	
	UCONbits.USBEN = 0; //disable usb module and transceiver to use RC4 and RC5
	UCFGbits.UTRDIS = 1; 

}




/******************************************************/





