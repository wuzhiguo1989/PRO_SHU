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
        
        // 工作信号 
        if(INT_PROC & UART_DUTY)
        {
            candidate_driver_signal_recieving = YES;
            driver_high_pin(key_cabinet_number);
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
    }
}


static void init_system_component() {  
    // timer
    init_system_tick();

    // ble
    Uart_Init();

    // DRIVER_IO
    driver_io_init();
}


// 默认柜锁关闭
static void init_logic_params() {
	u8 i;
	for(i = 1; i <= 28; i++)
		driver_low_pin(i);
}



