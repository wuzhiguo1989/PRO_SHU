#include "stc15wxxxx.h"
#include "tm1727.h"
#include "jl17467pqw.h"
#include "share.h"
#include "sync.h"
#include "task.h"
#include "light.h"
#include "adc.h"
#include "time.h"
#include "control.h"
#include "signal.h"
#include "beep.h"
#include "utils.h"

void main()
{
	display_clean();
	
	EA = 0;

	jt17467pqw_set_power_linked(1);

	light_io_init();

	init_alert();

	time_init();

	light_init();

	init_adc();

	control_init();

	init_control_signal();

	init_system_tick();

	init_system_tick1();

	init_beep_io();

	speak_init_params();

	init_speed_driver();

	EA =1;
	
  	while(1)
  	{
		reset_watch_dog();
		// 睡眠
		if(speak_mode == mute && state_current != NORMAL_STATE && state_current != KEY_NORMAL_STATE && !enter_lock_state_2s && !enter_lock_state_15s && !speak_mode_set) 
		{
			display_clean();
			PCON = 0x02;
		}
				
		// 遥控器
        if (INT_PROC & SIGNAL_RECEIVE_DUTY) {
        	handle_control_signal();
            INT_PROC &= ~SIGNAL_RECEIVE_DUTY;
            continue;
        }

        
        
		if(state_current == NORMAL_STATE || state_current == KEY_NORMAL_STATE)
		{
			

			// 控制器功率
			if (INT_PROC & CONTROL_RECEIVE_DUTY) {
				control_process_received(); 
        		INT_PROC &= ~CONTROL_RECEIVE_DUTY;
				continue;
			}
			// 10ms 刷新一次按键
			if (INT_PROC & KEY_SCAN_DUTY) {
				detect_buzzer_io();
				key_scan_task();
				//检测钥匙开关
				key_on_scan();
            	INT_PROC &= ~KEY_SCAN_DUTY;
            	continue;
        	}
        	// 1s同步一次时间
  			if (INT_PROC & TIME_SYNC_DUTY) {
				time_sync();
				INT_PROC &= ~TIME_SYNC_DUTY;
            	continue;
        	}


        	// 20ms 刷新一次屏幕
  			if (INT_PROC & LCD_DISPLAY_DUTY) {
				display(RAM_START_ADDRESS,display_code);
            	INT_PROC &= ~LCD_DISPLAY_DUTY;
            	continue;
        	}
        	if(INT_PROC & SPEED_CAL_DUTY){
				speed_cal_display();
				INT_PROC &= ~SPEED_CAL_DUTY;
        		continue;
        	}

        	// 250ms计算一次速度、温度、电池电压
        	if(INT_PROC & ADC_DUTY){
        		temperature_detected();
				battery_detected();
        		INT_PROC &= ~ADC_DUTY;
        		continue;
        	}

        	// 10ms 刷新一次按键
			if (INT_PROC & KEY_SCAN_DUTY) {
				detect_buzzer_io();
				key_scan_task();
				//检测钥匙开关
				key_on_scan();
            	INT_PROC &= ~KEY_SCAN_DUTY;
            	continue;
        	}
        }

	}
} 