#ifndef FUNCTS_H
#define FUNCTS_H

#include<p18cxxx.h>
#include "Pin_Definitions.h"
#include "i2c_lcd.h"
#include "SBC_Rpi.h"

void DELAY_1S(void);
void DELAY_500mS(void);
void DELAY_250mS(void);
void DELAY_100mS(void);
void DELAY_50mS(void);
void DELAY_1mS(void);
void DELAY_2mS(void);
void DELAY_10mS(void);

unsigned int ADC_Read(unsigned char);
void ADC_Init(void);
void Init_PowerInt(void);

unsigned char read_eeprom(unsigned char addr);
void write_eeprom(unsigned char Data, unsigned char addr );

#define POWER_INT_DISABLE  INTCON3bits.INT2IE=0
#define POWER_INT_ENABLE do{ INTCON3bits.INT2IF=0; INTCON3bits.INT2IE=1; }while(0)

void DisplayPressure(unsigned int adc_val);

struct cont_type {
     unsigned char qr_error;
};
extern struct cont_type cont;

void Tower_Write(unsigned char value);

#define TOWER_ON Tower_Write(0b00001000);
#define TOWER_OFF Tower_Write(0b00000000);

#endif
