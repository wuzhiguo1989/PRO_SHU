#ifndef __SHARE_H_
#define __SHARE_H_

#include "stc15wxxxx.h"
#include "def.h"

extern volatile bit time_colon_icon;

extern volatile time_t xdata local_time;

extern volatile u8 power_level;

extern volatile u8 color_level;

extern volatile u8 speak_mode_set;

extern volatile u8 status_mb_key; 

extern volatile bit enter_5s_countdown;

extern volatile bit countdown_signal_start;

extern volatile u8 charge_power;

extern volatile u8 power_error_code;

#endif
