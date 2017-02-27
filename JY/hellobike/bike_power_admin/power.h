#ifndef __POWER_H_
#define __POWER_H_
#include "utils.h"

// INT1
#define HILL_ASS P33
// INT0 
#define SIGN_IN  P32
// CHAR_DRIVER
#define CHAR_DRI P55

#define charge_power_disable() CHAR_DRI = 0
#define charge_power_enable()  CHAR_DRI = 1

typedef enum speed_mode_type{
	extreme_slow = 0,
	slow = 1,
	middle = 2,
	fast = 3,
	ultrahigh_fast = 4
} speed_t;

// extern volatile speed_t xdata speed;
extern volatile u8 speed;


void power_io_init();
void freq_cal_driver();
void driver_timer_enable();
void driver_timer_disable();

#endif
