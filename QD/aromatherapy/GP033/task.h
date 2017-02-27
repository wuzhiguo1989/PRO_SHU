#ifndef __TASK_H_
#define __TASK_H_

#include "config.h"

#define SLEEP_DUTY		0x01
#define TICK_DUTY       0X02
#define KEY_DUTY        0x04
#define UART_DUTY      0x08
#define PROTECT_DUTY        0x10


extern volatile u16 xdata INT_PROC;
#endif