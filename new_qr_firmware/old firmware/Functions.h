#ifndef FUNCTS_H
#define FUNCTS_H

#include<p18cxxx.h>

#include "Pin_Definitions.h"
#include "i2c_lcd.h"
//#include "ds1307.h"
#include "SBC_Rpi.h"

/** P R I V A T E   P R O T O T Y P E S ****************************/



void DELAY_1S(void);
void DELAY_500mS(void);
void DELAY_250mS(void);
void DELAY_100mS(void);
void DELAY_50mS(void);
void DELAY_1mS(void);
void DELAY_2mS(void);
void DELAY_10mS(void);

void LED_OFF(void);
void PASS_LED_ON(void);
void CLOG_LED_ON(void);
void LEAK_LED_ON(void);
void VALVE_LED_ON(void);

unsigned int ADC_Read(unsigned char);

void Valve_1_IN(void);
void Valve_1_OUT(void);

void Eject(void);


char  RtryValve_1_CW(void);
char RtryValve_1_ACW(void);
char  RtryValve_2_CW(void);
char RtryValve_2_ACW(void);

char RtryValve_1_FT(void);
char RtryValve_2_FT(void);

void RtryValve_1_CW_HF(void);
void RtryValve_1_ACW_HF(void);

void Step_1Sec_Clk2(void);

void ADC_Init(void);
void Init_PowerInt(void);


void board_test_protocol(void);
unsigned char read_eeprom(unsigned char addr);
void valve_left(void);
void valve_right_idle(void);
void write_eeprom(unsigned char Data, unsigned char addr );
unsigned char read_eeprom(unsigned char addr);

#define POWER_INT_DISABLE  INTCON3bits.INT2IE=0
#define POWER_INT_ENABLE INTCON3bits.INT2IF=0, INTCON3bits.INT2IE=1

void DisplayPressure(unsigned int adc_val);

extern unsigned char RtryValve_HF;

//bitfeild allowed values upto 15
struct cont_type {
	 unsigned char v1_cw_error:4;
	 unsigned char v1_acw_error:4;
	 
	 unsigned char v2_cw_error:4;
	 unsigned char v2_acw_error:4;
	 
	 unsigned char v1ft_cw_error:4;
	 unsigned char v1ft_acw_error:4;
	 
	 unsigned char v2ft_cw_error:4;
	 unsigned char v2ft_acw_error:4;
	 
	 unsigned char qr_error:4;
};
extern struct cont_type cont;

extern const char* sV1_CW_FAIL;
extern const char* sV1_ACW_FAIL;

extern const char* sV2_CW_FAIL;
extern const char* sV2_ACW_FAIL;

extern char sV1FT_ACW_FAIL[17];
extern char sV1FT_CW_FAIL[16];

extern char sV2FT_ACW_FAIL[17];
extern char sV2FT_CW_FAIL[16];

void Tower_Write(unsigned char value);

#define TOWER_ON Tower_Write(0b00001000);
#define TOWER_OFF Tower_Write(0b00000000);



#endif
