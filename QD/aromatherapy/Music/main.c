/*  
*   STC-ISP导入芯片信息至Keil
*   采用STC15W4K32S4系列
*   假定测试芯片的工作频率为24.000MHz
*/

#include "config.h"
#include "utils.h"
#include "sync.h"
#include "ble.h"
#include "task.h"
#include "key.h"
#include "display.h"
#include "string.h"
#include "usart.h"
#include "power.h"
#include "music.h"
#include "power.h"

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


    // begin catching interrupt
    EA = 1;

    while(1) {
        // Feed dog
        reset_watch_dog();
        if(INT_PROC & KEY_DUTY)
        {
            key_duty_task();
            // power_detect();
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
            if(power_status)
                power_level_8bit(power_level);
            else
                power_level_8bit(0);
            INT_PROC &= ~TICK_DUTY;
            continue;            
        }

    }
}


static void init_system_component() {  
    // timer
    init_system_tick();

    // ble
    BLE_Init();

    //pwm
    PWM_Init();
    PWM_Pin_Init();

    //KEY
    key_io_init();

    power_config_init();

}


// 默认柜锁关闭
static void init_logic_params() {
    init_sync_object();

    init_music_cotrol();

    display_startup_init();
}



