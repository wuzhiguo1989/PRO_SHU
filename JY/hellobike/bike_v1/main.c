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


    // begin catching interrupt
    EA = 1;
     
    INT_PROC = SLEEP_DUTY;   

    while(1) {
        // Feed dog
        reset_watch_dog();

        if(INT_PROC & KEY_DUTY)
        {
            count_down_at_all_task();
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
            cycle_based_control();
            led_display_task(); 
            INT_PROC &= ~TICK_DUTY;
            continue;            
        }

        if(INT_PROC & ALTER_DUTY)
        {
            // 上报报警信息
            speak_alter_pca_init();
            speak_beep_max();
            speak_mode = alter_beep;
            INT_PROC &= ~ALTER_DUTY;
            continue;
        }

        if(INT_PROC & SLEEP_DUTY)
        {
            EX1 = ENABLE;
            speak_mode = mute;
            speak_beep_disable();
            led_mode_set = led_display_off;
            led_display_stop();
            INT_PROC &= ~SLEEP_DUTY;
            PCON = 0x02;
            continue;
        }

        if(INT_PROC & LOCK_DUTY)
        {
            cancle_open_count_down();
            cancle_sleep_count_down();
            reload_lock_sleep_count_down();
            reload_lock_count_down();
            led_mode_set = lock;
            led_mode_changed(); 
            INT_PROC &= ~LOCK_DUTY;
            continue;
        }

        if(INT_PROC & LOCK_FAIL_DUTY)
        {
            EX1 = ENABLE;
            cancle_open_count_down();
            cancle_lock_count_down();
            cancle_sleep_count_down();
            led_flicker.cycle_enable = OFF;
            led_power_disable();
            status = on;
            cpu_wake_up_enable();
            uart_send_lock_by_hand_fail();
            cpu_wake_up_disable();
            reload_sleep_count_down();
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

//    init_adc();

    uart_Init();
	
}


static void init_logic_params() {
   init_sync_object();
}



