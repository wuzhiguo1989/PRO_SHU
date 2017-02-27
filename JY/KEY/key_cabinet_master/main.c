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
#include "share.h"
#include "adc.h"
#include "key.h"
#include "led.h"
#include "string.h"
#include "alarm.h"
#include "debug.h"
#include "usart.h"

static void init_system_component();
static void init_logic_params();

void Delay500ms()       //@24.000MHz
{
    unsigned char i, j, k;

    _nop_();
    _nop_();
    i = 46;
    j = 153;
    k = 245;
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
		
    // delay
    Delay500ms();

    instant_effective_init();


    // begin catching interrupt
    EA = 1;
    while(1) {
        // Feed dog
        reset_watch_dog();
        // 运行自检程序
        if(!JC)
        {
            key_cabinet_number = 1;
            Delay500ms();
            while(1)
            {
                u8 i;
                // Feed dog
                reset_watch_dog();
				if(JC == 1)
                {
                    Delay500ms();
                    performance_status = 0;
                    candidate_driver_signal_recieving = NO;
                    candidate_time_out_signal_recieving = NO;
                    INT_PROC = 0;
                    init_uart();
                    init_logic_params();
                    break;
                }
                for(i = 1; i <= 30; i++)
                {
                    if(i <= 28)
				    {
						putchar(i);
                        Delay500ms();
					}
				    else
					{
						driver_high_pin(i);
						Delay500ms();
						driver_low_pin(i);
					}                    
					while(1)
                    {
                        u32 key_mask = 0x00000001;
                        u32 key = IO_KeyScan_G() & 0x3fffffff;
                        // Feed dog
                        reset_watch_dog();
                        if(key & (key_mask << (i-1)))
                        {
                            display_error(0);
                            break;
                        }
                        else
                            display_error(i);
                    }
                }
            }
        }        
        // 数码管显示
        led_display();
        // 电源检测
		voltage_performance_detected();
        //TX1_write2buff(performance_status);
        // 柜门检测(5ms检测一次)
        if(INT_PROC & KEY_DUTY)
        {
            // if (USART_RX_Block(&COM1)) 
            //     INT_PROC |= UART_DUTY;
            key_duty_task();
            INT_PROC &= ~KEY_DUTY;
            continue;
        }

        if(INT_PROC & UART_DUTY)
        {
            handle_uart_data();
            INT_PROC &= ~UART_DUTY;
            continue;
        }

        // 柜门开锁持续时间到
        if(INT_PROC & DRIVER_DUTY)
        {
            driver_low_pin(key_cabinet_number);
            candidate_driver_signal_recieving = NO;
            reset_all_counters();          
            INT_PROC &= ~DRIVER_DUTY;
            continue;
        }

        // 5s等待时间到
        if(INT_PROC & TIME_OUT_DUTY)
        {
            // timeout
            candidate_time_out_signal_recieving = NO;
            //进入广播
            BLE_Enter_Adversting();
            INT_PROC &= ~TIME_OUT_DUTY;
            continue;
        }

    }
}


static void init_system_component() {  
    // timer
    init_system_tick();

    // ble
    BLE_Init();

    // 
    debug_init();
    
    // ADC
    init_adc();

    //KEY
    key_io_init();

    // DRIVER_IO
    driver_io_init();

    // DISPLAY_IO
    display_io_init();

    // ALARM_IO
    alarm_io_init();
}


// 默认柜锁关闭
static void init_logic_params() {
    driver_low_pin(29);
    driver_low_pin(30);
    ALARM = 0;
    memset(order_for_badkey, 0, 30);
}



