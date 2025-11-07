#include"i2c_lcd.h"

#define _XTAL_FREQ 48000000 



void display(char lcd_clear, ram char* line1, ram char* line2,unsigned char delay_1_sec){
	if(lcd_clear)LCD_Cmd(LCD_CLEAR);
	if (line1){
		LCD_Cmd(LCD_FIRST_ROW);
		LCD_Print_space(line1);
	}
	if (line2){
		LCD_Cmd(LCD_SECOND_ROW);
		LCD_Print_space(line2);
	}
	while(delay_1_sec--)DELAY_1S();

}

void display_ram(char lcd_clear, ram char* line1, ram char* line2,unsigned char delay_1_sec){
	if(lcd_clear)LCD_Cmd(LCD_CLEAR);
	if (line1){
		LCD_Cmd(LCD_FIRST_ROW);
		LCD_Print_rammem(line1);
	}
	if (line2){
		LCD_Cmd(LCD_SECOND_ROW);
		LCD_Print_rammem(line2);
	}
	while(delay_1_sec--)DELAY_1S();

}

void display_counts(void){
	extern unsigned long count, pass_count;
	unsigned long cur_count;
	char count_string[17];
      cur_count = count;
      count_string[0]='T';
      
      
      count_string[7] = cur_count % 10 + '0';
      cur_count = cur_count / (10);


      count_string[6] = cur_count % 10 + '0';
      cur_count = cur_count / (10);

      count_string[5] = cur_count % 10 + '0';
      cur_count = cur_count / (10);

      count_string[4 ] = cur_count % 10 + '0';
      cur_count = cur_count / (10);

      count_string[3 ] = cur_count % 10 + '0';
      cur_count = cur_count / (10);

      count_string[2 ] = cur_count % 10 + '0';
      cur_count = cur_count / (10);

      count_string[1 ] = cur_count % 10 + '0';
      cur_count = cur_count / (10);
      
      count_string[8 ] = '|';
      
      cur_count = pass_count;
      
      count_string[9 ] = 'P';
      
      count_string[15] = cur_count % 10 + '0';
      cur_count = cur_count / (10);


      count_string[14 ] = cur_count % 10 + '0';
      cur_count = cur_count / (10);

      count_string[13 ] = cur_count % 10 + '0';
      cur_count = cur_count / (10);

      count_string[12 ] = cur_count % 10 + '0';
      cur_count = cur_count / (10);

      count_string[11 ] = cur_count % 10 + '0';
      cur_count = cur_count / (10);

      count_string[10 ] = cur_count % 10 + '0';
      cur_count = cur_count / (10);

      
      cur_count = cur_count / (10);
	count_string[16]  = 0;
    LCD_Cmd(LCD_FIRST_ROW);
	LCD_Print_rammem(count_string);
}

void DisplayCatNotDet(void)
{
	unsigned int toggle=1500;
	TOWER_ON;
	while(SW_3){
		if(!(toggle%500)) {
			BUZZER=~BUZZER;
			
			if(toggle==1500){
				display(1," NO CARTRIDGE"," ",0);
				TOWER_OFF;
				
			}
			else if(toggle==3000){
				toggle=0;
				TOWER_ON;
			}
		}
		toggle++;	
		DELAY_2mS();
		DELAY_2mS();
		DELAY_2mS();	
	}
	TOWER_OFF;
	BUZZER=0;
	
}



void DisplayStackEmpty(void){
	unsigned int toggle=1500;
	while(1){
		if(!(toggle%500)) {
			BUZZER=~BUZZER;
			if(toggle==1500){
				display(1,"STACK EMPTY",0,0);
				
			}
			else if(toggle==3000){
			display(1,"FILL STACK &","PRESS START",0);
				toggle=0;
			}
		}
		toggle++;	
		DELAY_2mS();
		DELAY_2mS();
		DELAY_2mS();
		if(!SW_3){
			DELAY_100mS();
			if(!SW_3){
				break;
			}
		}
	}
	TOWER_OFF;
	BUZZER=0;
	
}

char RS;
uint8_t i2c_addr,backlight_val = LCD_BACKLIGHT;
	void I2C_Init1(void) {

    TRISBbits.TRISB1 = 0;
    TRISBbits.TRISB0 = 0;
    DELAY_2mS();
    LATBbits.LATB0=0;
    LATBbits.LATB1=0;
    DELAY_2mS();
    TRISBbits.TRISB1 = 1;
    TRISBbits.TRISB0 = 1;

    PIE1bits.SSPIE = 0;

    SSPSTAT = 0xC0; 
    SSPCON1 = 0x28; 
    SSPCON2 = 0;
    SSPADD = ((_XTAL_FREQ / 100000) / 4) - 1; 
    PIE1bits.SSPIE = 0; 
    PIR1bits.SSPIF = 0;
    
    
     
}


void I2C_Ready() {
    while (PIR2bits.BCLIF);


    while (SSPSTATbits.BF || (SSPSTATbits.R_W));
    PIR1bits.SSPIF = 0; 
}

void I2C_Start(void)
{
	while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F)); 
    SSPCON2bits.SEN = 1; 
    while (SSPCON2bits.SEN); 
    PIR1bits.SSPIF = 0;
    
    
    
}


void I2C_Stop(void)
{
	 I2C_Ready();
    SSPCON2bits.PEN = 1; 

    while (SSPCON2bits.PEN); 
    PIR1bits.SSPIF  = 0;
   
   
}





void I2C_Write(unsigned char Byte)
{
	 SSPBUF = Byte; 
    I2C_Ready();
    
}




void Expander_Write(uint8_t value)
{
  I2C_Start();
  I2C_Write(i2c_addr);
  I2C_Write(value | backlight_val);
  I2C_Stop();
}

void LCD_Write_Nibble(uint8_t n)
{
  n |= RS;
  Expander_Write(n & 0xFB);
  
	Delay10TCYx(2);
		
  Expander_Write(n | 0x04);
  
	Delay10TCYx(2);
	
  Expander_Write(n & 0xFB);
  Delay1KTCYx(5); 
}

void LCD_Cmd(uint8_t Command)
{
  RS = 0;
  LCD_Write_Nibble(Command & 0xF0);
  LCD_Write_Nibble((Command << 4) & 0xF0);
  if((Command == LCD_CLEAR) || (Command == LCD_RETURN_HOME))
    DELAY_2mS();DELAY_2mS();
}

void LCD_Goto(uint8_t col, uint8_t row)
{
  switch(row)
  {
    case 2:
      LCD_Cmd(LCD_SECOND_ROW + col - 1);
      break;
    default:
      LCD_Cmd(LCD_FIRST_ROW  + col - 1);
  }
}













void LCD_Print(rom char* LCD_Str)
{
  uint8_t i = 0;
  RS = 1;
  while(LCD_Str[i] != '\0')
  {
    LCD_Write_Nibble(LCD_Str[i] & 0xF0);
    LCD_Write_Nibble( (LCD_Str[i++] << 4) & 0xF0 );
  }
}


void LCD_Print_space(rom char* LCD_Str)
{
  char space=' ';
  uint8_t i = 0;
  RS = 1;

  while(LCD_Str[i] != '\0')
  {
    LCD_Write_Nibble(LCD_Str[i] & 0xF0);
    LCD_Write_Nibble( (LCD_Str[i++] << 4) & 0xF0 );
  }
  while(i<16){
  	LCD_Write_Nibble(space & 0xF0);
    LCD_Write_Nibble( (space << 4) & 0xF0 );
    i++;
  }
}


void LCD_Print_rammem(ram char* LCD_Str)
{
  uint8_t i = 0;
  RS = 1;
  while(LCD_Str[i] != '\0')
  {
    LCD_Write_Nibble(LCD_Str[i] & 0xF0);
    LCD_Write_Nibble( (LCD_Str[i++] << 4) & 0xF0 );
  }
}
void LCD_Begin(uint8_t _i2c_addr)
{
  i2c_addr = _i2c_addr;
  Expander_Write(0);

  DELAY_50mS();
  LCD_Cmd(3);
  DELAY_50mS();
  LCD_Cmd(3);
  DELAY_50mS();
  LCD_Cmd(3);
  DELAY_50mS();
  LCD_Cmd(LCD_RETURN_HOME);
  DELAY_50mS();
  LCD_Cmd(0x20 | (LCD_TYPE << 2));
  DELAY_50mS();
  LCD_Cmd(LCD_TURN_ON);
  DELAY_50mS();
  LCD_Cmd(LCD_CLEAR);
 DELAY_50mS();
  LCD_Cmd(LCD_ENTRY_MODE_SET | LCD_RETURN_HOME);
  DELAY_50mS();
}
