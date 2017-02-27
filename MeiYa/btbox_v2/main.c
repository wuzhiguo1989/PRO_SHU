#include "stc15wxxxx.h"
#include "share.h"
#include "sync.h"
#include "task.h"
#include "adc.h"
#include "time.h"
#include "utils.h"
#include "key.h"
#include "74hc595.h"


void Delay1000ms()		//@24.000MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 92;
	j = 50;
	k = 238;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

void main()
{
	EA = 0;
	time_init(); 
	Led_io_init();
	system_io_init();
	Source_clock_gen();
	init_system_tick();
	dispaly_driver_init();
	interrupt_control_init();
	status_system = 0;
	
	status_mb_key = 0;
	DisplayScanInit();

	Delay1000ms();

	Displayclean();

	init_adc();

	Source_Clock_Stop();


	EA = 1;



	// PCON = 0x02;
	// EX1 = ENABLE;
	
  	while(1)
  	{
		// reset_watch_dog();
		// 睡眠
		if(!status_system) 
		{
			Displayclean();
			PCON = 0x02;
		}
		if(status_system != 0)
		{
			if (INT_PROC & TICK_DUTY)
        	{
        		// delay interval response
            	delay_hold_interval();
            	count_down_task();
				INT_PROC &= ~TICK_DUTY;
            	continue;
        	}
		}
		if(status_system == 1 || status_system == 4 || status_system == 5)
		{
        	// 500ms 测电池电压
        	if(INT_PROC & ADC_DUTY){
        		power_level_cal();
        		INT_PROC &= ~ADC_DUTY;
        		continue;
        	}
        }
        if(status_system == 1 || status_system == 4 || status_system == 5)
        {
        // 10ms 刷新一次按键
			if (INT_PROC & KEY_DUTY) {
				key_duty_task();
				Display_digit_on_screen();
                if(adc_sampling)
                    no_work_exit();
            	INT_PROC &= ~KEY_DUTY;
            	continue;
        	}

			if (INT_PROC & REFRESH_DUTY) {
				DisplayScan();
            	INT_PROC &= ~REFRESH_DUTY;
            	continue;
        	}

            if(INT_PROC & SYSTEM_START_DUTY){
                reset_cnt_exit();

                // 欢迎使用美牙产品
                P31 = 1;
                P55 = 0;
                P12 = 1;
                
                start_work_detect();
                // 恢复显示电量
                power_display_level = 5;
                key_sampling = YES;
                // countdown_signal_start = YES;

                INT_PROC &= ~SYSTEM_START_DUTY;
                continue;
            }

    	}

    	if(status_system == 2)
    	{
			Displayclean();
			continue;
    	}

    	if(!CHG)	
    	{
        	if(!charge_on)
        	{
            	status_system = 5;
            	P23 = 0;
            	POUT = 0;
            	status_mb_key = 0;
            	EX0 = ENABLE;
            	EX1 = DISABLE;
            	time_colon_icon = 0;
                P31 = P12 = P55 = 1;
            	Source_Clock_Stop();
            	countdown_signal_start = NO;
            	time_init();
            	charge_on = 1;
        	}
    	}

    	if(status_system == 5)
    	{
    		if(CHG)
    		{
    			status_system = 0;
            	Displayclean();
				charge_on = 0;
            	countdown_signal_start = NO;
            	status_mb_key = 0;
            	// 关闭+、-灯
            	P23 = 0;
    		}
    	}

	}
} 