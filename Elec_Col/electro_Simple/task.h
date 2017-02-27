#ifndef __TASK_H_
#define __TASK_H_

#include "stc15wxxxx.h"

#define LCD_DISPLAY_DUTY		0x01
#define TIME_SYNC_DUTY 	        0x02
#define KEY_SCAN_DUTY           0x04
#define ADC_DUTY              	0x10
#define SPEED_CAL_DUTY 			0x40


extern volatile u8 xdata INT_PROC;
#endif