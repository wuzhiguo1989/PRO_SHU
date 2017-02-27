#ifndef __TASK_H_
#define __TASK_H_

#include "stc15wxxxx.h"

#define KEY_DUTY          		0x01
#define REFRESH_DUTY           	0x02
#define ADC_DUTY                0x04
#define TICK_DUTY               0x08
#define SYSTEM_START_DUTY       0x10
#define SLEEP_DUTY              0x20


extern volatile u8 xdata INT_PROC;
#endif