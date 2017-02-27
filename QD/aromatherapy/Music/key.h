#ifndef __KEY_H_
#define __KEY_H_

#include "config.h"

#define K1   P02
#define K2   P01

#define POWER_ID 0x02
#define LIGHT_ID 0x01


#define LONG_KEY_TIME   100
#define SHORT_KEY_TIME  25

void key_io_init();
u8 IO_KeyScan_G();
void key_scan_task();
void key_duty_task();

#endif