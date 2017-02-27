#ifndef __CONTROL_H_
#define __CONTROL_H_

#include "stc15wxxxx.h"

#define COM_RX2_Lenth   3
#define SPEED_CAL_COUNTER  8

void reset_speed_counter();

void control_init();

void mileage_init();

void control_process_received();

void speed_cal_display();

void Store_mileage();

#endif