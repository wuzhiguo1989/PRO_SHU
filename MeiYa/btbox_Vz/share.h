#ifndef __SHARE_H_
#define __SHARE_H_

#include "stc15wxxxx.h"
#include "def.h"

extern volatile bit time_colon_icon;

extern volatile time_t xdata local_time;

extern volatile bit countdown_signal_start;

extern volatile u8 status_system;

extern volatile u8 power_level;
extern volatile u8 power_display_level;

extern volatile u8 color_level;

extern volatile u8 status_mb_key;

extern volatile bit adc_sampling;
extern volatile bit key_sampling; 



extern volatile bit increase_fast_refresh;
extern volatile bit decrease_fast_refresh;
extern volatile u8 icon_power;

extern volatile bit charge_on;

extern volatile bit no_operation_exit;
extern volatile u16 interval_no_operation;




#endif
