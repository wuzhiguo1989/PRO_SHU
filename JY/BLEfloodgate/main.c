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
    
   // init_A();

    instant_effective_init();

    push_pull(DRIVER_Mx, DRIVER_BIT);
    // begin catching interrupt
    EA = 1;
    while(1) {
        // Feed dog
        reset_watch_dog(); 
        if(INT_PROC & BLE_DUTY)
        {
            ble_process_recieved(); 
            INT_PROC &= ~BLE_DUTY;
            continue;
        }
        // 闸机持续时间到
        if(INT_PROC & DRIVER_DUTY)
        {
              // 控制计时结束 
            candidate_driver_signal_recieving = NO;
            // 关锁
            DRIVER = 0;          
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

        if(INT_PROC & REQUEST_DUTY)
        {
            request_process_recieved();
            INT_PROC &= ~REQUEST_DUTY;
            continue;
        }        
    }
}


static void init_system_component() {  
    // timer
    init_system_tick();

    // ble
    BLE_Init();
}

static void init_logic_params() {
    DRIVER = 0;
}



