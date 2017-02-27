#include "typealias.h"
#include "config.h"
#include "uart.h"
#include "task.h"
#include "dwscreen.h"
#include "alert.h"
#include "adc.h"
#include "process.h"
#include "e2prom.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include "extension.h"

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

void main(void)
{
    init_io_alert();
    init_adc();
    valve_control_io_init();
    uart_init();
    system_tick_init();
    workflow_init();
    dw_init();
    alert_beep_disable(); 
    count_down_init_all_task();
    Extern0_Init();
    EA = 1;
    Timer0_Run();
    tem_around = temperature_around_cal();
    tem_fan = temperature_fan_cal();
    send_two_value_to_screen(ADDR_TEMPERATURE_AROUND, tem_around, tem_fan);
	password_init();
    param_init();
    Delay1000ms();
	dw_screen_init();
    Delay5ms();
    system_enter_idle();
    available_system_set();
    while (1)
	{
        
        reset_watch_dog();
        if (INT_PROC & DW_DUTY) {
            printf("DW_DUTY, PoW\r\n");

        //    dw_read_data_from_screen();
            INT_PROC &= ~DW_DUTY;
            continue;
        }
        if (INT_PROC & WF_FINISH_DUTY) {
            printf("WF_FINISH_DUTY, PoW\r\n");
            wf_finish_duty_task();
            INT_PROC &= ~WF_FINISH_DUTY;
            continue;
        }

        if(INT_PROC & START_DUTY)
        {
            syc_signal_start_fan(1);
            printf("START\r\n");
            count_down_cancle_all_task();
            valve_disable();
            detected_feed_full_start();
            INT_PROC &= ~START_DUTY;
            continue;
        }
        // 报警产生关闭
        if(INT_PROC & SYSTEM_END_DUTY)
        {
            printf("END\r\n");
            fan_work_disable();
            count_down_cancle_all_task();
            valve_disable();
            system_enter_idle();
            start_fan_signal = 0;
            syc_signal_start_fan(0);
            INT_PROC &= ~SYSTEM_END_DUTY;
            continue;
        }
        // 5ms system tick-tock
        if (INT_PROC & TICK_DUTY) {
            count_down_all_task();
            if (UART_RX_Block(&COM2)) 
                call_for_duty(COM2_RX_DUTY);
            INT_PROC &= ~TICK_DUTY;
            continue;
        }

        if (INT_PROC & COM2_RX_DUTY) {
            // int len = dw_cache_response(RX2_Buffer);
            // printf("COM2_RX_DUTY, PoW\r\n");
            // UART_Finish_Rx(&COM2);
            // dw_dispatch_response(dw_cached_response, len);
			int len = dw_cache_response(RX2_Buffer);
            // printf("COM2_RX_DUTY, PoW\r\n");
            UART_Finish_Rx(&COM2);
            dw_dispatch_response(dw_cached_response, len);
			if (simple_workflow[COM2_ID].status == processing && simple_workflow[COM2_ID].callback != NULL) 
            {
                simple_workflow[COM2_ID].callback();
                simple_workflow[COM2_ID].status = wait_for_finish;
                call_for_duty(S_WF_DUTY);
            }
            INT_PROC &= ~COM2_RX_DUTY;
            continue;
        }
        if (INT_PROC & WF_FLOW_DUTY) {
            wf_flow_duty_task();
            INT_PROC &= ~WF_FLOW_DUTY;
            continue;
        }

        if (INT_PROC & S_WF_DUTY) {
            s_wf_duty_task();
            INT_PROC &= ~S_WF_DUTY;
            continue;
        }
        if (INT_PROC & SECOND_DUTY) {
            // dw_gen_task();
            // wf_chk_timeout_duty_task();
            // s_wf_chk_timeout_duty_task();
            alert_for_temperature();
            alert_for_overload();
            alert_beep();
            INT_PROC &= ~SECOND_DUTY;
            continue;
        }
        if(INT_PROC & FEED_SUCTION_END_DUTY)
        {
            fan_work_disable();
            printf("FEED_SUCTION_END_DUTY\r\n");
            count_down_cancle_all_task();
            //人工关机
            if(!start_fan_signal){
                syc_signal_start_fan(0);
                printf("idle\r\n");
                system_enter_idle();
            } else {
                // 故障
                if(alert_for_system & 0x14)
                {
                    system_enter_idle();
                    // 同步按钮
                    start_fan_signal = 0;
                    syc_signal_start_fan(0);
                }
                // 料满
                if(alert_for_system & ALTER_FULL_FEED)
                    system_enter_alter_feed_full();
            }
            feed_suction_callback();
            INT_PROC &=  ~FEED_SUCTION_END_DUTY;
            continue;
        }
        if(INT_PROC & ALTER_BEEP_DUTY)
        {
            alert_beep_enable();
            syc_signal_beep();
            INT_PROC &= ~ALTER_BEEP_DUTY;
            continue;
        }
        if(INT_PROC & ALTER_BEEP_CLOSE_DUTY)
        {
            alert_beep_disable();
            INT_PROC &= ~ALTER_BEEP_CLOSE_DUTY;
            continue;
        }
	}
}



