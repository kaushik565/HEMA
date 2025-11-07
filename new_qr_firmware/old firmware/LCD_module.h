#include <p18cxxx.h>
#include "xlcd.h"
#include <delays.h>
#include "SBC_Rpi.h"
// #include "DS2782_FuelG.h"  // Battery fuel gauge - not needed for cartridge jig

void mainlcd (void);
void DisplayLoading(void);
void DisplayNext(void);
void DisplayMesProcess(void);
void DisplayHeating(void);
void DisplayPressure(unsigned int adc_val);
void DisplayPressure_idle(void);
void DisplayMesADDW4(void);
void Display_Sample(void);
void Display_WashA1 (void);
void Display_WashA2 (void);
void Display_WashA3 (void);
void Display_WashB1 (void);
void Display_WashB2 (void);
void Display_WashB3 (void);
void Display_WashB4 (void);
void Display_Wash2 (void);





void Display_Wash3 (void);
void Display_Elution (void);
void DisplayBlank2(void);
void DisplayBlank3(void);
void DisplayBlank4(void);
void DisplayFinished(void);		//10i
void DisplayTimerValue(unsigned int);	//10i
void DisplayLoadError(void);
void DisplayBinding(void);
void DisplayWash1 (void);
void DisplayWash2 (void);
void DisplayWash3 (void);
void DisplayWash4 (void);
void DisplayWash5 (void);
void DisplayWash6 (void);
void DisplayElution (void);
void Display_Col_Elute(void);
void DisplayPressure2(void);
void DisplayHeaterTest1(void);
void DisplayHeaterTest2(void);
void DisplaySW_Pause(void);

void DisplayCartError (void);
void DisplayCloggedError (void);
void BasePressureError (void);
void DisplayDockError (void);

void Display_ValveError(void);

void Display_Count(void);
void Display_Count2(void);
void RTD_ERROR (void);

void InsertCart (void);

void LowBattWarning(void);
void Display_BuffError1(void);
void Display_BuffError2(void);
void Save_Settings(void);

