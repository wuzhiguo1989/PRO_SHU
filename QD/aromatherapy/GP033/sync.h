#ifndef __SYNC_H_
#define __SYNC_H_

#include "config.h"

#define KEY_SCAN_TASK_INTERVAL  4

typedef struct cycle_control_entity
{
	u8 cycle_output;
	u8 cycle_enable;
	u16 counter_for_cycle;
} cycle_control_t;

extern volatile bit countdown_signal_start;
extern volatile cycle_control_t xdata led_flicker;


void init_sync_object();
void cycle_based_control();
void init_system_tick();
void sync_with_alert();
void countdown_off(u8 time_index);


#endif