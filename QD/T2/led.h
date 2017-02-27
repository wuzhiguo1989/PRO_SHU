#ifndef __LED_H_
#define __LED_H_

#include "stc15w408as.h"
#include "def.h"

// flag set to 0
// clear set to 1
#define  WDT_FLAG	0x80
#define  CLR_WDT	0x10


// sbit  CCP0      =   P1^0;
// sbit  CCP1      =   P1^1;
// sbit  CCP2      =   P1^2;
//#define bulb_output_pin P11
#define CCP0 P55
//#define CCP1 P54
//#define CCP2 P10
#define BT_CTS P33
#define BT_RTS P10
#define MCU_CS P54
#define reset_watch_dog()     WDT_CONTR &= ~WDT_FLAG;	WDT_CONTR |= CLR_WDT
#endif
