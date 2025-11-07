#include "LCD_module.h"
#include "Functions.h"


void mainlcd (void)
{

    unsigned char *row1=" TRUEPREP AUTO  ";
//	unsigned char *row2="  & PRS START   ";
//	unsigned char *row3="     DEVICE     ";

	XLCDInit();
	XLCDPutRomString(row1);
//	XLCDL2home();
//	XLCDPutRomString(row2);
//	XLCDL3home();
//	XLCDPutRomString(row3);
	Nop();
}

void InsertCart (void)
{
	char	DisplayString[17];
	unsigned	char	i=9;
	char string[18]="INSERT CARTRIDGE";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';
	XLCDL1home();
	XLCDPutRamString(string);

}


void	DisplayInit()
{
	char	DisplayString[17];
	unsigned	char	i=9;
//char string[25]="Initializing... ";
  char string[25]="DEVICE INIT...ZN";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';
	XLCDL2home();;
	XLCDPutRamString(string);
}

void	DisplayStart()
{
	char	DisplayString[17];
	unsigned	char	i=9;
	char string[25]="  Denaturation  ";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';
	XLCDL2home();
	XLCDPutRamString(string);
}

void	Displaytest(unsigned int	Val)
{
	char	DisplayString[17];
	unsigned	char	i, Valmsb, Vallsb,Valnsb;
//	char string[12]="Cycle No ";
	char string[12]="Rec SPs ";
	for(i=0; string[i]!='\0'; i++);
	
	Valmsb=Val/100;
	Valnsb=((Val/10)%10);
	Vallsb=Val%10;
	string[i++] = Valmsb+0x30;
	string[i++] = Valnsb+0x30;
	string[i++] = Vallsb+0x30;
	string[i++] = '\0';
	XLCDL2home();
	XLCDPutRamString(string);
}

void DisplayLoading()
{
	char	DisplayString[17];
    unsigned char *row3="    Cartridge...";
	unsigned	char	i=9;
	char string[18]="Loading         ";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';
	XLCDInit();
	XLCDL3home();
	XLCDPutRamString(string);
	XLCDL4home();
	XLCDPutRomString(row3);
}

void DisplayNext()
{
	//char	DisplayString[17];
	unsigned	char	i=9;
	char string[18]="PRESS NEXT      ";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';
	XLCDL4home();
	XLCDPutRamString(string);

}

void DisplayMesProcess()
{
	//char	DisplayString[17];
	unsigned	char	i=9;
	char string[18]="---PROCESSING---";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';
	XLCDL4home();
	XLCDPutRamString(string);
/*	#ifdef SBC_ENABLED
		write_rom_rpi('\n');
		write_ram_string_rpi(string);
		write_rom_rpi('\n');
	#endif*/
}




void DisplayHeating()
{
	char uart_sting[5];
	unsigned	char	i=9;
	unsigned char first_digit,second_digit, third_digit, fourth_digit;
	extern unsigned int TM;
	char string[18]="HEATING   T=   s";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';


	first_digit= (int) 	TM/100;
	second_digit=((TM)%100)/10;
	third_digit=(TM)%10;

		string[12] =first_digit+0x30;
		string[13] =second_digit+0x30;
		string[14] =third_digit+0x30;

	#ifdef SBC_ENABLED
		uart_sting[0]=string[12] =first_digit+0x30;
		uart_sting[1]=string[13] =second_digit+0x30;
		uart_sting[2]=string[14] =third_digit+0x30;
		uart_sting[3]=',';
		uart_sting[4]=0;
		write_ram_string_rpi(uart_sting);
	#endif
	
	XLCDL2home();
	XLCDPutRamString(string);
}

void DisplayBlank2()
{
	char	DisplayString[18];
	unsigned	char	i=9;
	char string[18]="                ";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';
	XLCDL2home();
	XLCDPutRamString(string);
}
void DisplayBlank3()
{
	char	DisplayString[18];
	unsigned	char	i=9;
	char string[18]="                ";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';
	XLCDL3home();
	XLCDPutRamString(string);
}
void DisplayBlank4()
{
	char	DisplayString[18];
	unsigned	char	i=9;
	char string[18]="                ";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';
	XLCDL4home();
	XLCDPutRamString(string);
}
void DisplayPressure_idle(void)
{
	char	DisplayString[17];
	unsigned char i, Valmsb, Vallsb,Valnsb,Valxsb,Valasb;
	double input_vol,output_vol,bat_vol;
	unsigned char first_digit,second_digit, third_digit, fourth_digit;
	char string[18]="P=        T=   S"; 
	unsigned long voltage_value;
	unsigned int output,Val;
	long mult_10=0;
	extern unsigned int C_BaseValue;
	extern unsigned int TM;
	extern unsigned char battery_percent;

/*********************************/
//	init_adc_thermistor();
	C_BaseValue=ADC_Read(0);
	output_vol=C_BaseValue;
	voltage_value=output_vol/.2046;	
/********************************/
	first_digit=(int) voltage_value/1000;
	second_digit=((voltage_value)%1000)/100;
	third_digit=((voltage_value)%100)/10;
	fourth_digit=(voltage_value)%10;

	string[2] =first_digit+0x30;
	string[3] ='.';
	string[4] =second_digit+0x30;
	string[5] =third_digit+0x30;
	string[6] =fourth_digit+0x30;
	string[7]='V';

//	read_fuel_guage();
	first_digit= (int) 	TM/100;
	second_digit=((TM)%100)/10;
	third_digit=(TM)%10;

	string[12] =first_digit+0x30;
	string[13] =second_digit+0x30;
	string[14] =third_digit+0x30;


	XLCDL2home();
	XLCDPutRamString(string);
}

void DisplayPressure()
{
	char	SBC_String[12];
	unsigned char i, Valmsb, Vallsb,Valnsb,Valxsb,Valasb;
	double input_vol,output_vol,bat_vol;
	unsigned char first_digit,second_digit, third_digit, fourth_digit;
	char string[18]="P=        T=   s"; 
	unsigned long voltage_value;
	unsigned int output,Val;
	long mult_10=0;
	extern unsigned int C_BaseValue;
	extern unsigned int TM;

/*********************************/
//	init_adc_thermistor();
	C_BaseValue=ADC_Read(0);
	output_vol=C_BaseValue;
	voltage_value=output_vol/.2046;	
/********************************/
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

/********************************/
#ifdef SBC_ENABLED
	write_ram_string_rpi(SBC_String);
#endif
/********************************/

	XLCDL2home();
	XLCDPutRamString(string);
}


void DisplayPressure2()
{
	char	DisplayString[17];
	unsigned char i, Valmsb, Vallsb,Valnsb,Valxsb,Valasb;
	double input_vol,output_vol,bat_vol;
	unsigned char first_digit,second_digit, third_digit, fourth_digit;
	char string[18]="Base Pres=      "; 
	unsigned long voltage_value;
	unsigned int output,Val;
	long mult_10=0;
	extern unsigned int BaseValue;

/*********************************/
//	init_adc_thermistor();
//	output_vol=ADC_Read(1);
	output_vol=BaseValue;
	voltage_value=output_vol/.2046;	
/********************************/
	first_digit=(int) voltage_value/1000;
	second_digit=((voltage_value)%1000)/100;
	third_digit=((voltage_value)%100)/10;
	fourth_digit=(voltage_value)%10;

	string[10] =first_digit+0x30;
	string[11] ='.';
	string[12] =second_digit+0x30;
	string[13] =third_digit+0x30;
	string[14] =fourth_digit+0x30;
	string[15]='V';


	XLCDL4home();
	XLCDPutRamString(string);
}



void DisplayMesADDW4()
{
	extern unsigned int kj;
	char	DisplayString[17];
	unsigned char i, Valmsb, Vallsb,Valnsb,Valxsb,Valasb;
	double input_vol,output_vol,bat_vol;
	unsigned char first_digit,second_digit, third_digit, fourth_digit;
	char string[18]="PR SENSOR=      "; 
	unsigned long voltage_value;
	unsigned int output,Val;
	long mult_10=0;

/*********************************/
//	init_adc_thermistor();
	output_vol=kj;
	voltage_value=output_vol/.2046;	
/********************************/
	first_digit=(int) voltage_value/1000;
	second_digit=((voltage_value)%1000)/100;
	third_digit=((voltage_value)%100)/10;
	fourth_digit=(voltage_value)%10;

	string[10] =first_digit+0x30;
	string[11] ='.';
	string[12] =second_digit+0x30;
	string[13] =third_digit+0x30;
	string[14] =fourth_digit+0x30;
	string[15]='V';

/********************************/

write_ram_string_rpi(string);
write_rom_rpi('\n');
/********************************/

	XLCDL3home();
	XLCDPutRamString(string);
}

void Display_Sample()
{
	//char	DisplayString[18];
	unsigned	char	i=9;
	char string[18]="Step:1    Sample";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';
	XLCDL1home();
	XLCDPutRamString(string);

}

void Display_WashA1()
{
	//char	DisplayString[18];
	unsigned	char	i=9;
	char string[18]="Step:2  Wash A:1";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';
	XLCDL1home();
	XLCDPutRamString(string);
	write_rom_rpi('\n');
	write_ram_string_rpi(string);
	write_rom_rpi('\n');
}
void Display_WashA2()
{
	//char	DisplayString[18];
	unsigned	char	i=9;
	char string[18]="Step:2  Wash A:2";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';
	XLCDL1home();
	XLCDPutRamString(string);
	write_rom_rpi('\n');
	write_ram_string_rpi(string);
	write_rom_rpi('\n');
}
void Display_WashA3()
{
	//char	DisplayString[18];
	unsigned	char	i=9;
	char string[18]="Step:2  Wash A:3";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';
	XLCDL1home();
	XLCDPutRamString(string);
	write_rom_rpi('\n');
	write_ram_string_rpi(string);
	write_rom_rpi('\n');
}

void Display_WashB1()
{
	//char	DisplayString[18];
	unsigned	char	i=9;
	char string[18]="Step:3  Wash B:1";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';
	XLCDL1home();
	XLCDPutRamString(string);
	write_rom_rpi('\n');
	write_ram_string_rpi(string);
	write_rom_rpi('\n');
}
void Display_WashB2()
{
	//char	DisplayString[18];
	unsigned	char	i=9;
	char string[18]="Step:3  Wash B:2";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';
	XLCDL1home();
	XLCDPutRamString(string);
	write_rom_rpi('\n');
	write_ram_string_rpi(string);
	write_rom_rpi('\n');
}
void Display_WashB3()
{
	//char	DisplayString[18];
	unsigned	char	i=9;
	char string[18]="Step:3  Wash B:3";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';
	XLCDL1home();
	XLCDPutRamString(string);
	write_rom_rpi('\n');
	write_ram_string_rpi(string);
	write_rom_rpi('\n');
}

void Display_WashB4()
{
	//char	DisplayString[18];
	unsigned	char	i=9;
	char string[18]="Step:3  Wash B:4";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';
	XLCDL1home();
	XLCDPutRamString(string);
	write_rom_rpi('\n');
	write_ram_string_rpi(string);
	write_rom_rpi('\n');
}

void Display_Wash3()
{
	//char	DisplayString[18];
	unsigned	char	i=9;
	char string[18]="Step:3    Wash:3";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';
	XLCDL1home();
	XLCDPutRamString(string);
	write_rom_rpi('\n');
	write_ram_string_rpi(string);
	write_rom_rpi('\n');

}

void Display_Elution()
{
	//char	DisplayString[18];
	unsigned	char	i=9;
	char string[18]="Step:4   Elution";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';
	XLCDL1home();
	XLCDPutRamString(string);
	
	write_rom_rpi('\n');
	write_ram_string_rpi(string);
	write_rom_rpi('\n');
}

void DisplayFinished()	//10i
{
	//char	DisplayString[18];
	unsigned	char	i=9;
	char string[18]="---COMPLETED----";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';
	XLCDL2home();
	XLCDPutRamString(string);
	write_rom_rpi('\n');
	write_ram_string_rpi(string);
	write_rom_rpi('\n');
}

void DisplayTimerValue(unsigned int time2)
{
	char string[18]="    SECONDS     ";
	unsigned char *firstrow="    REMAINING...";
	unsigned char hsb,msb;
	hsb=(time2/100);	
	msb=((time2/10)%10);
	if(hsb==0)
	string[0]=' ';
	else
	string[0]=hsb+0x30;
	if(msb==0 & hsb==0)
	string[1]=' ';
	else
	string[1]=msb+0x30;
	string[2]=(time2%10)+0x30;
	string[16] = '\0';
	XLCDL3home();
	XLCDPutRamString(string);
	XLCDL4home();
	XLCDPutRomString(firstrow);
	INTCONbits.TMR0IF=0;
}


void DisplaySW_Pause(void)
{
	unsigned int Value=0x00;
  	unsigned char *firstrow="PRS STR TO PAUSE";
	XLCDL4home();
	XLCDPutRomString(firstrow);
	Nop();
}

void DisplayLoadError (void)
{
	char	DisplayString[17];
    unsigned char *row3="          Loaded";
	unsigned	char	i=9;
	char string[18]="Cartridge Not   ";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';
	XLCDL1home();
	XLCDPutRamString(string);
	XLCDL2home();
	XLCDPutRomString(row3);
}

void DisplayDockError (void)
{
	char	DisplayString[17];
    unsigned char *row3="           Error";
	unsigned char i=9;
	char string[18]="Cartridge       ";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';
	XLCDL3home();
	XLCDPutRamString(string);
	XLCDL4home();
	XLCDPutRomString(row3);
}
void DisplayWash1 (void)
{
	unsigned int Value=0x00;
    unsigned char *firstrow="     WASH-1     ";
	XLCDInit();
	XLCDPutRomString(firstrow);
	Nop();
}


void DisplayWash2 (void)
{
	unsigned int Value=0x00;
    unsigned char *firstrow="     WASH-2     ";
	XLCDInit();
	XLCDPutRomString(firstrow);
	Nop();
}


void DisplayWash3 (void)
{
	unsigned int Value=0x00;
    unsigned char *firstrow="     WASH-3     ";
	XLCDInit();
	XLCDPutRomString(firstrow);
	Nop();
}


void DisplayWash4 (void)
{
	unsigned int Value=0x00;
    unsigned char *firstrow="    WASH-4      ";
	XLCDInit();
	XLCDPutRomString(firstrow);
	Nop();
}

void DisplayWash5 (void)
{
	unsigned int Value=0x00;
    unsigned char *firstrow="    WASH-5      ";
	XLCDInit();
	XLCDPutRomString(firstrow);
	Nop();
}

void DisplayWash6 (void)
{
	unsigned int Value=0x00;
    unsigned char *firstrow="    WASH-6      ";
	XLCDInit();
	XLCDPutRomString(firstrow);
	Nop();
}

void DisplayElution (void)
{
	unsigned int Value=0x00;
    unsigned char *firstrow="     ELUTION    ";
	XLCDInit();
	XLCDPutRomString(firstrow);
	Nop();
}
void DisplayCartError (void)
{
	unsigned int Value=0x00;
    unsigned char *firstrow="Cartridge Error ";
    unsigned char *thirdrow="   Press Eject  ";
	unsigned char *fourthrow="         to Exit";
	XLCDInit();
	XLCDPutRomString(firstrow);
	XLCDL2home();
	XLCDPutRomString(thirdrow);
	XLCDL3home();
	XLCDPutRomString(fourthrow);
	Nop();
}

void DisplayHeaterTest1 (void)
{
	unsigned int Value=0x00;
    unsigned char *firstrow="Heater Testing E";
    unsigned char *thirdrow="    Elution     ";
	unsigned char *fourthrow="    ";
	XLCDInit();
	XLCDPutRomString(firstrow);
	XLCDL2home();
	XLCDPutRomString(thirdrow);
//	XLCDL4home();
//	XLCDPutRomString(fourthrow);
	Nop();
}
void DisplayHeaterTest2 (void)
{
	unsigned int Value=0x00;
    unsigned char *firstrow="Heater Testing L";
    unsigned char *thirdrow="     Lysis      ";
	unsigned char *fourthrow="    ";
	XLCDInit();
	XLCDPutRomString(firstrow);
	XLCDL2home();
	XLCDPutRomString(thirdrow);
//	XLCDL4home();
//	XLCDPutRomString(fourthrow);
	Nop();
}
void DisplayCloggedError (void)
{
	unsigned int Value=0x00;
    unsigned char *firstrow="CartridgeClogged";
    unsigned char *thirdrow="   Press Eject  ";
	unsigned char *fourthrow="         to Exit";
	XLCDInit();
	XLCDPutRomString(firstrow);
	XLCDL2home();
	XLCDPutRomString(thirdrow);
	XLCDL4home();
	XLCDPutRomString(fourthrow);
	Nop();
}

void BasePressureError (void)
{
	unsigned int Value=0x00;
    unsigned char *firstrow="Self Test Failed";
    unsigned char *Secondrow="Press Eject     ";
	unsigned char *thirdrow="      to Restart";
//	XLCDInit();
	XLCDL1home();
	XLCDPutRomString(firstrow);
	XLCDL2home();
	XLCDPutRomString(Secondrow);
//	XLCDL3home();
//	XLCDPutRomString(thirdrow);
	Nop();
}

void Display_Count(void)
{

	unsigned int A=0,B=0;
	char string[18]= "TC=     EC=     "; 
	char string1[17]="Firmware Ver:8.0"; 
	char string2[17]="LC=     CC=     ";
	char string3[17]="      "; 
	char string4[17]="Buffer Count=   "; 
	extern unsigned int BF_Count;
	extern union
	{
		unsigned int bit16;
		unsigned char bit8[2];
	}count,Leak_Count,Clog_Count,Sl_Count;

	XLCDInit();
	A=Sl_Count.bit16/10000;
	string[3] =A+0x30;
	B=Sl_Count.bit16%10000;
	A=B/1000;
	string[4] =A+0x30;
	B=B%1000;
	A=B/100; 
	string[5] =A+0x30;
	B=B%100;
	A=B/10; 
	string[6] =A+0x30;
	A=B%10; 
	string[7] =A+0x30;

	A=count.bit16/10000;
	string[11] =A+0x30;
	B=count.bit16%10000;
	A=B/1000;
	string[12] =A+0x30;
	B=B%1000;
	A=B/100; 
	string[13] =A+0x30;
	B=B%100;
	A=B/10; 
	string[14] =A+0x30;
	A=B%10; 
	string[15] =A+0x30;

	A=Leak_Count.bit16/10000;
	string2[3] =A+0x30;
	B=Leak_Count.bit16%10000;
	A=B/1000;
	string2[4] =A+0x30;
	B=B%1000;
	A=B/100; 
	string2[5] =A+0x30;
	B=B%100;
	A=B/10; 
	string2[6] =A+0x30;
	A=B%10; 
	string2[7] =A+0x30;

	A=Clog_Count.bit16/10000;
	string2[11] =A+0x30;
	B=Clog_Count.bit16%10000;
	A=B/1000;
	string2[12] =A+0x30;
	B=B%1000;
	A=B/100; 
	string2[13] =A+0x30;
	B=B%100;
	A=B/10; 
	string2[14] =A+0x30;
	A=B%10; 
	string2[15] =A+0x30;
	
	A=BF_Count/10;
	string4[13] =A+0x30;
	A=BF_Count%10;
	string4[14]=A+0x30;
	XLCDL1home();
	XLCDPutRamString(string1);
	XLCDL2home();
	XLCDPutRamString(string4);

	DELAY_1S();
	DELAY_1S();
	XLCDL1home();
	XLCDPutRamString(string);
	XLCDL2home();
	XLCDPutRamString(string2);

}

void Display_Count2(void)
{

	unsigned int A=0,B=0;

	char string4[17]="Buffer Count=   "; 
	extern unsigned int BF_Count;
	extern union
	{
		unsigned int bit16;
		unsigned char bit8[2];
	}count,Leak_Count,Clog_Count,Sl_Count;

	XLCDInit();	
	A=BF_Count/10;
	string4[13] =A+0x30;
	A=BF_Count%10;
	string4[14]=A+0x30;

	XLCDL2home();
	XLCDPutRamString(string4);

	DELAY_1S();
	DELAY_1S();
}


void RTD_ERROR (void)
{
 	char string[18]="   RTD ERROR    ";
	XLCDInit();
	XLCDL2home();
	XLCDPutRamString(string);	

}

void Display_ValveError(void)
{
 	char string[18]="  VALVE ERROR   ";
 	char string2[18]="  PRESS EJECT   ";
	XLCDInit();
	XLCDPutRamString(string);
	XLCDL2home();
	XLCDPutRamString(string2);
}
void Display_BuffError1(void)
{
 	char string1[17]=" Change Reagent ";
 	char string2[17]="  Pack & Reset  ";
	XLCDInit();
	XLCDPutRamString(string1);
	XLCDL2home();
	XLCDPutRamString(string2);
}
void Display_BuffError2(void)
{
 	char string1[17]="  By Pressing   ";
 	char string2[17]="Start & Eject SW";
	XLCDInit();
	XLCDPutRamString(string1);
	XLCDL2home();
	XLCDPutRamString(string2);
}

void LowBattWarning(void)
{
   	unsigned char *row1="  Battery Low ! ";
	unsigned char *row2="Connect Charger ";
//	unsigned char *row3="     DEVICE     ";

	XLCDInit();
	XLCDPutRomString(row1);
	XLCDL2home();
	XLCDPutRomString(row2);

}

	
void Display_Col_Elute()
{
	char	DisplayString[17];
	unsigned	char	i=9;
	char string[18]=" Collect Elute  ";
	for(i=11; string[i]!='\0'; i++);
	string[i++] = '\0';
	XLCDL1home();
	XLCDPutRamString(string);
}

void Save_Settings()
{
	char string1[17]= " Settings Saved ";
	XLCDInit();
	XLCDPutRamString(string1);

}



