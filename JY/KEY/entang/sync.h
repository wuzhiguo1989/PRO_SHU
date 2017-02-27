#ifndef __SYNC_H_
#define __SYNC_H_

#include "config.h"

#define COUNTER_PER_SECOND 		200
#define COUNTER_PER_INTERVAL	60
#define DRIVER_TASK_INTERVAL	1
#define TIME_OUT_TASK_INTERVAL 	5

void init_system_tick();
void reset_all_counters();
void sync_with_alert();
void reset_counter_for_ble();

#define reset_counter_for_second_time_out() counter_for_second_time_out = COUNTER_PER_SECOND
#define reset_counter_for_second_driver() counter_for_second_driver = COUNTER_PER_SECOND
#define reset_counter_for_timeout() counter_for_timeout = TIME_OUT_TASK_INTERVAL
#define reset_counter_for_driver() counter_for_driver = DRIVER_TASK_INTERVAL
#endif