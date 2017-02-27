#include "stc15wxxxx.h"
#include "share.h"
#include "sync.h"
#include "task.h"
#include "adc.h"
#include "time.h"
#include "utils.h"
#include "key.h"
#include "74hc595.h"

void main()
{
	
	EA = 0;
	init_adc();
	key_io_init();
	Led_io_init();
	init_system_tick();
	Timer2Init();
	time_init();
	speak_init_params();
	DisplayScanInit();

	EA =1;
	
  	while(1)
  	{
		reset_watch_dog();

        // 500ms计算一次速度、温度、电池电压
        if(INT_PROC & ADC_DUTY){
			power_error_code = power_detect_in();
			// power_error_code = 0;
        	power_level_cal();
        	color_level ++;
        	if(color_level == 25)
        		color_level = 0;
        	INT_PROC &= ~ADC_DUTY;
        	continue;
        }

        // 10ms 刷新一次按键
		if (INT_PROC & KEY_DUTY) {
			key_duty_task();
			Display_digit_on_screen();
            INT_PROC &= ~KEY_DUTY;
            continue;
        }

		if (INT_PROC & REFRESH_DUTY) {
			DisplayScan();
            INT_PROC &= ~REFRESH_DUTY;
            continue;
        }

	}
} 