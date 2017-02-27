#ifndef __TASK_H_
#define __TASK_H_

#include "stc15wxxxx.h"

#define LCD_DISPLAY_DUTY		0x01
#define TIME_SYNC_DUTY 	        0x02
#define KEY_SCAN_DUTY           0x04
#define CONTROL_RECEIVE_DUTY    0x08
#define ADC_DUTY              	0x10
#define SIGNAL_RECEIVE_DUTY     0x20
#define SPEED_CAL_DUTY 			0x40
#define ALERT_DUTY				0x80


extern volatile u8 xdata INT_PROC;
#endif