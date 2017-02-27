#ifndef __SYNC_H_
#define __SYNC_H_

#include "led.h"
                                
#define exint_disable() EX0 = 0
#define exint_enable() EX0 = 1

void init_sync_timer();
void init_ac_driver();
void init_phase_sync_timer();

#define COUNTER_PER_SECOND 200
#define SECONDS_PER_MINUTE 10

void reset_all_counters();

#define reset_counter_for_second() counter_for_second = COUNTER_PER_SECOND
#define reset_counter_for_minute() counter_for_minute = SECONDS_PER_MINUTE
#define reset_counter_for_hertz() counter_for_hertz = 0;
#define reset_counter_for_cycle() counter_for_cycle = 0;

#define phase_ctrl_enable()  CR = 1
//TR0 = 1; ET0 = 1 	
#define phase_ctrl_disable()  CR = 0
//TR0 = 0; ET0 = 0

#define T200Hz  (FOSC / 12 / 200)
#define T4kHz	(FOSC / 12 / 4000)
// [20] * T4KHz = T200Hz
#define PCA5ms	20

//#define PHASE_WIDTH		0xFD1F	// 480us
//#define PHASE_WIDTH		0xFD9A	// 400us
//#define PHASE_WIDTH		0xFDD7	// 360us
//#define PHASE_WIDTH			0xFECD	// 200us
#define PHASE_WIDTH			(FOSC / 12 / 4000)	// 250us


#define CCP_S0 0x10                 //P_SW1^4
#define CCP_S1 0x20                 //P_SW1^5

// wait for bluetooh power-on alert
void sync_with_alert();

#define cycle_ctrl_enable() TR0= 1; ET0 = 1
#define cycle_ctrl_disable() TR0= 0; ET0 = 0

#endif