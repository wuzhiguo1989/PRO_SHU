#include "stc15wxxxx.h"
#include "share.h"
#include "utils.h"

volatile bit time_colon_icon = 0;

volatile time_t xdata local_time;


volatile u8 power_level = 0;

volatile u8 color_level = 0;

volatile u8 speak_mode_set = 0;

volatile u8 status_mb_key = 0; 

volatile bit enter_5s_countdown = NO;

volatile bit countdown_signal_start = NO;

volatile u8 charge_power = 0;

volatile u8 power_error_code = 4;