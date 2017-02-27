#ifndef __INTERACT_H_
#define __INTERACT_H_

#include "stc15wxxxx.h"
#include "utils.h"

          
// TIME should not over 255 OR please check CntPlus datatype
#define LONG_KEY_TIME   40
#define SHORT_KEY_TIME  5

// extern u8 g_ShortKeyCode; //短按键,单次触发 
// extern u8 g_LongKeyCode;  //长按键,持续触发 

void key_scan_task();
void key_task_duty();

#endif