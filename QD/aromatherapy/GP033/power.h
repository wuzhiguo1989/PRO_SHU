#ifndef __POWER_H_
#define __POWER_H_

#include	"config.h"
#include	"PWM.h"

#define POWER_8BIT_4LEVEL 4
#define POWER_IN  P00
#define POWER_OUT P32

typedef unsigned char power_level_t;
extern volatile bool power_status;
extern volatile bool sys_status;
extern volatile power_level_t xdata power_level_set;
void power_level_8bit(const power_level_t xdata power_level);
void power_select_level();
void power_config_init();
void power_overload_protect();

#endif
