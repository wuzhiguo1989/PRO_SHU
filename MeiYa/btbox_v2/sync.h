#ifndef __SYNC_H_
#define __SYNC_H_

#include "stc15wxxxx.h"
#include "def.h"


#define T500Hz	(MAIN_Fosc / 12 / 500)

#define Source_Clock_Run() AUXR |= 0x10
#define Source_Clock_Stop() AUXR &= ~0x10  

void init_system_tick();
void Source_clock_gen();
void interrupt_control_init();
void dispaly_driver_init();
void delay_hold_interval();
void reset_work_start();

void count_down_task();

#endif