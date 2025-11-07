#include "Functions.h"

struct cont_type cont;

void Tower_Write(unsigned char value)
{
#ifdef BOARD_VER2
  I2C_Start();
  I2C_Write(0b01110010);
  I2C_Write(value);
  I2C_Stop();
#endif
}

unsigned char read_eeprom(unsigned char addr){
	unsigned char data;
	EEADR = addr; 
	EECON1 &= 0x3F; 
	EECON1 |= 0x01; 
	while ( EECON1 & 0x01 ); 
	data = EEDATA; 
	return data;
}

void write_eeprom(unsigned char Data, unsigned char addr ) {
	EEADR = addr; 
	EEDATA = Data; 
	EECON1 &= 0x3F; 
	EECON1 |= 0x04; 
	INTCONbits.GIE = 0; 
	EECON2 = 0x55; 
	EECON2 = 0xAA; 
	EECON1 |= 0x02; 
	INTCONbits.GIE = 1; 
	while ( EECON1 & 0x20 ); 
	 DELAY_50mS();
	EECON1 &= 0xFB; 
}


void DELAY_10mS(void)
{
	unsigned int i,j;
	for(j=0;j<30;j++)				
	for (i = 0; i < 73; i++);
}
void DELAY_1S(void)
{	
	unsigned int i,j;
	for(j=0;j<80;j++)				
	for (i = 0; i < 9980; i++);
}

void DELAY_500mS(void)
{
	unsigned int i,j;
	for(j=0;j<28;j++)			
	for (i = 0; i < 9920; i++);
}

void DELAY_250mS(void)
{
	unsigned int i,j;
	for(j=0;j<14;j++)				
	for (i = 0; i < 9920; i++);
}
 
void DELAY_100mS(void)
{	
	unsigned int i,j;
	for(j=0;j<6;j++)					
	for (i = 0; i < 9265; i++);
}
void DELAY_50mS(void)
{
	unsigned int i,j;
	for(j=0;j<3;j++)				
	for (i = 0; i < 9000; i++);
}
void DELAY_1mS(void)
{
	unsigned int i,j;
	for(j=0;j<3;j++)				
	for (i = 0; i < 65; i++);
}
void DELAY_2mS(void)
{
	unsigned int i,j;
	for(j=0;j<3;j++)				
	for (i = 0; i < 180; i++);
}

void ADC_Init(void){
	ADCON1=0x0B;
	ADCON2=0x35;
	ADCON0=0x00;
}

unsigned int ADC_Read(unsigned char ChnlNo)
{
	unsigned int data1=0x00,data2=0x00,data=0x00,loop=0,data_send=0x00,loop_count=20;
	unsigned long int data_tot=0x00;

	switch (ChnlNo)
	{	
		case 0:
		ADCON0=0b00000001;
		break;
		case 1:
		ADCON0=0b00000101;
		break;
		case 2:
		ADCON0=0b00001001;
		break;
		case 3:
		ADCON0=0b00001101;
		break;
		case 4:
		ADCON0=0b00010001;
		break;
	}

	while(loop<loop_count)		
	{
	loop++;						
	ADCON0bits.GO=0x01;			
	Nop();						
	while(ADCON0bits.GO);
	data1=ADRESH;			
	data2=ADRESL;			
	data1=data1<<2;			
	data2=data2>>6;			
	data=data1|data2;		
	data_tot=data_tot+data;	
	Nop();					
	Nop();					
	Nop();					
	Nop();					
	}
	data_send=(data_tot/loop_count);
	return(data_send);				
}

void Step_1Sec_Clk2(void)
{
	unsigned int i;

	for(i=0;i<650;i++)
	{
		CLK_2=1;
		DELAY_2mS();
		CLK_2=0;
		DELAY_2mS();
	}

}



void Step_1Sec_Clk4(void)
{
	unsigned int i;

	for(i=0;i<325;i++)
	{
		CLK_2=1;
		DELAY_2mS();DELAY_2mS();
		CLK_2=0;
		DELAY_2mS();DELAY_2mS();
	}

}


void Init_PowerInt(void){
	INTCON2bits.INTEDG2=0;                     
	INTCON3bits.INT2IF=0;                       
	INTCON3bits.INT2IP=1;  
	INTCON3bits.INT2IE=1; 
	INTCONbits.GIE=1;
}




	#ifdef BOARD_TEST
void read_current_test(unsigned char adc,unsigned int current_adc_offset){

	char c;
	unsigned char first_digit,second_digit, third_digit, fourth_digit;
	unsigned int current_adc=0,current_adc_avg=0,current_v=0, current_c=0,current_adc_min=1023 ,current_adc_max=0;
	char string[17]="Current=      ";  
	while(SW_1){
		current_adc_avg=0;
				for(c=0;c<20;c++){
					current_adc=ADC_Read(adc);
					current_adc_avg+=current_adc;					
				}
				current_adc_avg=current_adc_avg/20;
				if(current_adc_offset>current_adc_avg){
				current_c = (((unsigned long int)current_adc_offset) - current_adc_avg);
					
				current_c =((unsigned long int)current_c*((unsigned long int)1000))/((unsigned long int)1023);
				
					string[0+8] ='-';
				 	string[1+8] = ((int) current_c / 100) + '0';
				    string[2+8] = (((current_c) % 100) / 10) + '0';
				    string[3+8] = ((current_c) % 10) + '0';
				    string[4+8] = ' ';
				    string[5+8] = 'm';
				    string[6+8] = 'A';			
				    string[7+8] = 0;
				    
				     LCD_Cmd(LCD_SECOND_ROW );
				   	 LCD_Print_rammem(string);
				}
				else if(current_adc_avg>current_adc_offset){
					current_c = (((unsigned long int)current_adc_avg) - current_adc_offset);
				
				
					current_c =((unsigned long int)current_c*((unsigned long int)1000))/((unsigned long int)1023);
				
					string[0+8] ='+';
				 	string[1+8] = ((int) current_c / 100) + '0';
				    string[2+8] = (((current_c) % 100) / 10) + '0';
				    string[3+8] = ((current_c) % 10) + '0';
				    string[4+8] = ' ';
				    string[5+8] = 'm';
				    string[6+8] = 'A';
				    string[7+8] = 0;

				    LCD_Cmd(LCD_SECOND_ROW );
				   	 LCD_Print_rammem(string);
				   
				   
				}
				else {
					
					current_c = 0;
					string[0+8] =  ' ';
				    string[1+8] = '0';
				    string[2+8] = '0';
				    string[3+8] = '0';
				    string[4+8] = ' ';
				    string[5+8] = 'm';
				    string[6+8] = 'A';
				    string[7+8] = 0;
				 
				    LCD_Cmd(LCD_SECOND_ROW );
				   	 LCD_Print_rammem(string);
				}
		
			}

}
	#endif
void DisplayPressure(unsigned int adc_val)
{
char	SBC_String[12];
	double output_vol;
	unsigned char first_digit,second_digit, third_digit, fourth_digit;
	char string[18]="P=        T=   s"; 
	unsigned long voltage_value;
	extern unsigned int C_BaseValue;
	extern unsigned int TM;



	C_BaseValue=adc_val;
	output_vol=C_BaseValue;
	voltage_value=output_vol/.2046;	

	first_digit=(int) voltage_value/1000;
	second_digit=((voltage_value)%1000)/100;
	third_digit=((voltage_value)%100)/10;
	fourth_digit=(voltage_value)%10;
	SBC_String[3]=',';
	SBC_String[4]=string[2] =first_digit+0x30;
	SBC_String[5]=string[3] ='.';
	SBC_String[6]=string[4] =second_digit+0x30;
	SBC_String[7]=string[5] =third_digit+0x30;
	SBC_String[8]=string[6] =fourth_digit+0x30;
	string[7]='V';
	SBC_String[9]='\n';
	SBC_String[10]=0;
	
	first_digit= (int) 	TM/100;
	second_digit=((TM)%100)/10;
	third_digit=(TM)%10;

	SBC_String[0]=string[12] =first_digit+0x30;
	SBC_String[1]=string[13] =second_digit+0x30;
	SBC_String[2]=string[14] =third_digit+0x30;


	if(sbc_ready==1){
	write_ram_string_rpi(SBC_String);
	}



	LCD_Cmd(LCD_SECOND_ROW );
	LCD_Print_rammem(string);
}












































































































































































































































































































































































































































