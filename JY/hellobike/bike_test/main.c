/*  
*   STC-ISP导入芯片信息至Keil
*   采用STC15W4K32S4系列
*   假定测试芯片的工作频率为24.000MHz
*/

#include "utils.h"
#include "sync.h"
#include "task.h"
#include "key.h"
#include "string.h"
#include "power.h"
#include "lm567.h"
#include "utils.h"
#include "usart.h"
#include "beep.h"
#include "interact.h"

#define TIME_PLUS 100

static void init_system_component();
static void init_logic_params();


void Delay1000ms()      //@24.000MHz
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


void instant_effective_init() {
    init_system_component();
    init_logic_params();

}


void main()
{  
    // stop catching interrupt
    EA = 0;

    instant_effective_init();
    count_down_init_all_task();
    k_power_enable();

    // begin catching interrupt
    EA = 1;
     
    // INT_PROC = SLEEP_DUTY;   

    while(1) {
        // Feed dog
        reset_watch_dog();

        if(INT_PROC & KEY_DUTY)
        {
            count_down_at_all_task();
            key_task_duty();
            send_adc();
            INT_PROC &= ~KEY_DUTY;
            continue;
        }

        if(INT_PROC & UART_DUTY) 
        { 
            uart_process_recieved(); 
            INT_PROC &= ~UART_DUTY;              
            continue; 
        } 


        // 5ms检测一次
        if(INT_PROC & TICK_DUTY)
        {
            if (USART_RX_Block(&COM1))  
                INT_PROC |= UART_DUTY;
            // if (!lock_on_detect_signal){
            //     if(status_lock_on_detect() > TIME_PLUS){
            //         power_work_disable();
            //         // 开锁成功上报
            //         uart_send_open_success();
            //         status = on;
            //         cnt_on_time_plus = 0;
            //         lock_on_detect_signal = 1;
            //         k_power_disable();
            //         cancle_open_count_down();
            //     }
            // }
            // if (!lock_off_detect_signal){
            //     if(status_lock_off_detect() > TIME_PLUS){
            //         power_work_disable();
            //         EX1 = ENABLE;
            //         // 关锁成功上报
            //         uart_send_lock_by_hand_success();
            //         status = off;
            //         cnt_off_time_plus = 0;
            //         lock_off_detect_signal = 1;
            //         resend_lock_command_signal = 1;
            //         k_power_disable();
            //         //reload_sleep_count_down();
            //         cancle_lock_count_down();
            //         // 倒计时一秒等待回应
            //         reload_resend_lock_command();    
            //     }
                
            // }
            cycle_based_control();
            led_display_task(); 
            INT_PROC &= ~TICK_DUTY;
            continue;            
        }

        if(INT_PROC & LOCK_DETECT_DUTY){
            // reload_sleep_count_down();
            INT_PROC &= ~LOCK_DETECT_DUTY;
            continue;
        }


        // if(INT_PROC & SLEEP_DUTY)
        // {
        //     EX1 = ENABLE;
        //     k_power_disable();
        //     speak_mode = mute;
        //     speak_beep_disable();
        //     led_mode_set = led_display_off;
        //     led_display_stop();
        //     status_in_sleep = 0;
        //     INT_PROC &= ~SLEEP_DUTY;
        //     PCON = 0x02;
        //     continue;
        // }

        if(INT_PROC & LOCK_DUTY)
        {
            power_lock_work();
            reload_lock_count_down();
            reload_lock_beep_count_down();
            lock_off_detect_signal = 0;
            cnt_off_time_plus = 0;
            led_mode_set = lock;
            lock_signal = 1;
            led_mode_changed(); 
            INT_PROC &= ~LOCK_DUTY;
            continue;
        }

        if(INT_PROC & LOCK_FAIL_DUTY)
        {
            cancle_open_count_down();
            cancle_lock_count_down();
            led_flicker.cycle_enable = OFF;
            led_power_disable();
            status = on;
            uart_send_lock_by_hand_fail();
            resend_lock_command_signal = 2;
            // reload_sleep_count_down();
            reload_resend_lock_command();
            EX1 = ENABLE;
            INT_PROC &= ~LOCK_FAIL_DUTY;
            continue;
        }
    }
}


static void init_system_component() {  
    // timer
    init_system_tick();

		
	// power
	power_io_init();

    alter_beep_protect();

    speak_init_params();
    
    init_beep_io();

    led_io_init();

   init_adc();

    uart_Init();
	
}


static void init_logic_params() {
   init_sync_object();
}



