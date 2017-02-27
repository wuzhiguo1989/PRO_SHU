#ifndef __SHARE_H_
#define __SHARE_H_

#include "stc15wxxxx.h"
#include "def.h"

#define LOCK_STATE    0
#define UN_LOCK_STATE 1
#define ALERT_STATE   2
#define NORMAL_STATE  3
#define KEY_NORMAL_STATE 4



extern volatile u8 display_code[20];
extern volatile bit time_colon_icon;


extern volatile time_t xdata local_time;


extern volatile u8 speak_mode_set; 
extern volatile speak_t speak_mode;
extern volatile u16 speak_time;
extern volatile u8 interval;

extern volatile u16 speed_counter;

extern volatile u16 local;  

#endif
