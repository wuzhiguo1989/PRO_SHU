#ifndef     __ALARM_H_
#define     __ALARM_H_


#include "config.h"



#define ALARM  P50

#define alarm_for_no_problem()     ALARM = 0

void alarm_io_init();

void alarm_for_batter_low();

void alarm_for_220v_lost();

void alarm_for_key_cabinet();


#endif