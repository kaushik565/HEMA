#include "service.h"
rom char string[17]="OK      >      <";
typedef void (*item_callback_type)(void);
extern unsigned int TM;
extern unsigned int BaseValue,C_BaseValue;

#define Product_Pasword 4751
#define Pasword_Pos 5

static void menu(unsigned char items, char** menu_string, item_callback_type* function) {
    unsigned char item=0;
    display(1,menu_string[item],string,0);
    while(!SW_1);
    while(1) {
        if(!SW_3) {
            item++;
            if(item==items) item=0;   
            display(1,menu_string[item],string,0);
            while(!SW_3);
        }
        else if(!SW_2) {
            if(item) item--;
            else item=items-1;
            display(1,menu_string[item],string,0);
            while(!SW_2);
        }
        else if(!SW_1) {
            while(!SW_1);
            if(item==(items-1))return;
            else function[item]();
            display(1,menu_string[item],string,0);
            while(!SW_1);
        }
        DELAY_100mS();
    }
}

void mech_error_botton(void){
	while (SW_3&&SW_2){ TOWER_OFF; BUZZER=1; DELAY_500mS(); TOWER_ON; BUZZER=0; DELAY_500mS(); }
	TOWER_OFF;
}

void mech_error_loop(void){
	while(1){ TOWER_OFF; BUZZER=1; DELAY_500mS(); TOWER_ON; BUZZER=0; DELAY_500mS(); }
}

static void mechanism_test(void){ display(1,"MECH TEST",0,0); display(0,0,"NOT REQUIRED",0); DELAY_1S(); DELAY_1S(); }
static void bluetooth(void) { char in=0; write_rom_rpi(0); wait_ready_rpi(); if(sbc_ready){ display(1,"PAIR",0,0); write_rom_rpi(24); while(SW_3 || SW_2) { if (PIR1bits.RCIF == 1) { PIR1bits.RCIF = 0; in = RCREG; if(in=='R'){ display(1,"OK",0,2); break; } if(in=='T'){ display(1,"T-OUT",0,2); break; } } } } else { display(1,"ER",0,0); } DELAY_1S(); }

void service_menu(void) {
    char main_menu_items=3;
    char* main_menu_string[3]= {"MECH TST","BT PAIR","EXIT"};
    item_callback_type item_callback[2]= {mechanism_test, bluetooth};
    POWER_INT_DISABLE; display(1,"MENU",0,0); DELAY_1S(); while(!SW_2);
    menu(main_menu_items,main_menu_string,item_callback);
}
