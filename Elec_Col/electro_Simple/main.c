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
#include "beep.h"
#include "utils.h"

void main()
{
	display_clean();
	
	EA = 0;

	jt17467pqw_set_power_linked(1);

	time_init();

	init_adc();

	control_init();

	init_key_io();

	init_system_tick();


	init_beep_io();

	speak_init_params();

	init_speed_driver();

	EA =1;
	
  	while(1)
  	{
		reset_watch_dog();
			// 10ms 刷新一次按键
			if (INT_PROC & KEY_SCAN_DUTY) {
				// detect_buzzer_io();
				key_scan_task();
				temperature_sample();
				battery_sample();
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

        	// 500ms计算一次速度、温度、电池电压
        	if(INT_PROC & ADC_DUTY){
        		temperature_detected();
				battery_detected();
				speed_cal_display();
        		INT_PROC &= ~ADC_DUTY;
        		continue;
        	}
        }

}