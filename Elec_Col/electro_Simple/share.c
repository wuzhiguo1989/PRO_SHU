#include "stc15wxxxx.h"
#include "share.h"
#include "utils.h"
volatile u8 display_code[20]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};


volatile bit time_colon_icon = 1;

volatile time_t xdata local_time;

volatile u8 speak_mode_set = 0; 

volatile speak_t speak_mode; 

volatile u16 speed_counter = 0; 

volatile u16 local; 

