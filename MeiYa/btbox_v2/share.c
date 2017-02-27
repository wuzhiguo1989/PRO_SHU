#include "stc15wxxxx.h"
#include "share.h"
#include "utils.h"

volatile bit time_colon_icon = 1;

volatile time_t xdata local_time;

volatile bit countdown_signal_start = NO;


volatile u8 status_system = 0;


volatile u8 power_level = 5;
volatile u8 power_display_level = 5;

volatile u8 color_level = 0;


volatile u8 status_mb_key = 0; 

volatile bit increase_fast_refresh = NO;
volatile bit decrease_fast_refresh = NO;
volatile u8 icon_power = 0;

volatile bit charge_on = 0;
volatile bit no_operation_exit = NO;


volatile bit adc_sampling = NO;
volatile bit key_sampling = NO;

volatile u16 interval_no_operation = 6000;

