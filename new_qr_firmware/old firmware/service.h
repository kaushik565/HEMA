#include<p18cxxx.h>

#include "Pin_Definitions.h"
#include "i2c_lcd.h"
//#include "ds1307.h"
#include "SBC_Rpi.h"
#include "Functions.h"

void service_menu(void);
char read_current(unsigned char adc,char auto_flag,unsigned char time);
void Password_set(void);
void mech_error_botton(void);
void mech_error_loop(void);
