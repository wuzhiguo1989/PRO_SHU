/*  
*   STC-ISP导入芯片信息至Keil
*   采用STC15W4K32S4系列
*   假定测试芯片的工作频率为24.000MHz
*/

#include "config.h"
#include "utils.h"
#include "sync.h"
#include "task.h"
#include "key.h"
#include "display.h"
#include "string.h"
#include "power.h"
#include "LM567.h"
#include "utils.h"
#include "usart.h"
#include "led.h"

static void init_system_component();
static void init_logic_params();



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

    // power_select_level();
    // begin catching interrupt

    EA = 1;
        

    while(1) {
        // Feed dog
        reset_watch_dog();
        if(INT_PROC & KEY_DUTY)
        {
            key_duty_task();
            decrease_key_hold_interval();
            count_down_at_all_task();
            // power_select_level();
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

        if(INT_PROC & SLEEP_DUTY){
            // 倒计时
            // 关灯、关香薰
            power_status = OFF;
            power_select_level();
            ctrl_mode_changed(0);
            INT_PROC &= ~SLEEP_DUTY;
            EX0 = ENABLE;
            continue; 
        }

        if(INT_PROC & PROTECT_DUTY){
            power_level_set = 0;
            power_select_level();
            INT_PROC &= ~PROTECT_DUTY;
            continue; 
        }

    }
}


static void init_system_component() {  
    // timer
    init_system_tick();
    // Timer1Init();
    //KEY
    key_io_init();
    power_config_init();

    //pwm
    PWM_Init();
	PWM_Pin_Init();
    LED_OUT = 0;

    // // INT0
    power_overload_protect();
    init_adc();
    BLE_Init();
	
}


static void init_logic_params() {
    init_sync_object();
    display_startup_init();
}



