#ifndef __SYNC_H_
#define __SYNC_H_

#include "stc15wxxxx.h"

#define LCD_DISPLAY_TASK_INTERVAL 25
#define COUNTER_PER_SECOND 		50
#define SPEED_TASK_INTERVAL  	50

void reset_speed_cal();
void reset_time_counter();
void reset_speed_time();

void init_system_tick();
void init_speed_driver();

#endif