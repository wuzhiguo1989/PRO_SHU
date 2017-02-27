#include "stc15wxxxx.h"
#include "share.h"
#include "utils.h"
volatile u8 display_code[20]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

//0：解锁 ————    ————
//			  ————
//1：寻车
//2：关锁
//3：报警1 4s
//4：报警2 15s
//5：启动  1s
volatile u8 speak_time_interval[7] ={50, 200, 10, 10, 10, 50, 50};

volatile bit time_colon_icon = 1;

volatile time_t xdata local_time;

volatile bit enter_lock_state_2s = NO;
volatile bit enter_lock_state_15s = NO;
volatile bit first_alert = 0;

volatile u8 state_current = LOCK_STATE;



volatile u16 speak_mode_set = 0; 

volatile speak_t speak_mode; 


volatile u16 speak_time = 0;
volatile u8 interval = 0;

volatile u16 speed_counter = 0;  

