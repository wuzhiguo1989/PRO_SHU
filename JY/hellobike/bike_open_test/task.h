#ifndef __TASK_H_
#define __TASK_H_
#include "stc15wxxxx.h"

#define LOCK_DETECT_DUTY		0x01
#define TICK_DUTY       0X02
#define KEY_DUTY        0x04
#define UART_DUTY       0x08
#define SLEEP_DUTY      0x10
#define LOCK_DUTY       0x20
#define LOCK_FAIL_DUTY  0x40
#define UART_SEND_DUTY 0x80
#define UART1_SEND_DUTY 0x0100

extern volatile u16 xdata INT_PROC;
#endif