#ifndef __SYNC_H_
#define __SYNC_H_

#include "stc15wxxxx.h"
#include "def.h"

#define KEY_SCAN_TASK_INTERVAL 100
#define COUNTER_PER_SECOND     200
#define SPEED_TASK_INTERVAL  100
#define COUNTDOWN 1000


void reset_countdown_speak(); 
void reset_countdown_5s();
void init_system_tick();
void Timer2Init(); 

#endif