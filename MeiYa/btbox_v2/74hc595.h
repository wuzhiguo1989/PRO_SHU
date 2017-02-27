#ifndef __74HC595_H_
#define __74HC595_H_

#include "stc15wxxxx.h"
#include "def.h"

// MCU pin display
#define DISPLAY_D0  P22
#define DISPLAY_D1  P21
#define DISPLAY_D2  P20
#define DISPLAY_D3  P37
#define DISPLAY_D4  P36
#define DISPLAY_D5  P35
#define DISPLAY_D6  P34
//  com
#define DISPLAY_C0  P11
#define DISPLAY_C1  P10
#define DISPLAY_C2  P27
#define DISPLAY_C3  P26
#define DISPLAY_C4  P25
#define DISPLAY_C5  P24

#define DISPLAY_D(i)  DISPLAY_D##i
#define DISPLAY_C(i)  DISPLAY_C##i

void  Led_io_init(void);
void  DisplayScanInit(void);
void  Display_digit_on_screen(void);
void Displayclean(void);
void  DisplayScan(void);

#endif