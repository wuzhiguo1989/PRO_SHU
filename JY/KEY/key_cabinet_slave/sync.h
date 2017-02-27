#ifndef __SYNC_H_
#define __SYNC_H_

#include "config.h"

#define DRIVER_TASK_INTERVAL 60

void init_system_tick();
void reset_all_counters();

#define reset_counter_for_driver() counter_for_driver = DRIVER_TASK_INTERVAL

#endif