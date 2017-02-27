#ifndef __KEY_H_
#define __KEY_H_

#include "stc15wxxxx.h"

#define CHG   P15
#define POUT  P54

#define LONG_KEY_TIME   20
#define SHORT_KEY_TIME  4

extern volatile u8 g_ShortKeyCode;    //短按键,单次触发 
extern volatile u8 g_LongKeyCode;     //长按键,持续触发


void system_io_init();
void key_scan_task();
void key_duty_task();

#endif