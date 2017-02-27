#include "config.h"
#include "utils.h"
#include "sync.h"
#include "ble.h"
#include "task.h"
#include "share.h"
#include "entang.h"

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
    while(1)
    {
        reset_watch_dog(); 

        //BLE_Enter_Adversting();

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
    // io口
    quasi_bidirectional(2, 0x04);
    quasi_bidirectional(2, 0x08);
    push_pull(DRIVER_Mx, DRIVER_BIT);
    push_pull(3, 0x10);
    pure_input(3, 0x04);  
    // timer
    init_system_tick();

    // ble
    BLE_Init();
}

static void init_logic_params() {
    DRIVER = 0;
    BC = 1;
    ENTANG_key_init();
}



