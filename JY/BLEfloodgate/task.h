#ifndef __TASK_H_
#define __TASK_H_

#include "config.h"

#define BLE_DUTY		0x01
#define DRIVER_DUTY 	0x02
#define TIME_OUT_DUTY   0X04
#define REQUEST_DUTY    0X08


extern volatile u8 xdata INT_PROC;
#endif