#ifndef __SYNC_H_
#define __SYNC_H_

#include "led.h"
                                
#define exint_disable() EX0 = 0
#define exint_enable() EX0 = 1
void init_sync_timer();
void init_ac_driver();
// void init_counter_sync_timer();
void reset_all_counters();

#define reset_counter_for_cycle() counter_for_cycle = 0;


#define T200Hz  (FOSC / 12 / 200)
#define T40kHz	(FOSC / 12 / 40000)
// [20] * T4KHz = T200Hz
#define PCA5ms	4

#endif