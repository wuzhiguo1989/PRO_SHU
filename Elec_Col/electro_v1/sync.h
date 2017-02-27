#ifndef __SYNC_H_
#define __SYNC_H_

#include "stc15wxxxx.h"

#define LCD_DISPLAY_TASK_INTERVAL 25
#define COUNTER_PER_SECOND 		50
#define SPEED_TASK_INTERVAL  	50
#define COUNTER_FOR_SECOND_2S   200
#define COUNTER_FOR_SECOND_10S  1000

#define init_alert()  INT_CLKO |= 0x10

void reset_2s_counter();
void reset_2s_counter_for_key();
void reset_10s_counter();
void reset_time_counter();

void init_system_tick();
void init_system_tick1();
void init_speed_driver();

#endif