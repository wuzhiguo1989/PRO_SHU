#include "sync.h"
#include "task.h"
#include "share.h"
#include "utils.h"
#include "config.h"

#define T200Hz  (MAIN_Fosc / 12 / 200)
#define Timer0_Reload   (65536UL - T200Hz)  

static u8 counter_for_second = 200;
static u8 counter_for_reset = 60;
static u8 counter_for_second_time_out = COUNTER_PER_SECOND;
static u8 counter_for_second_driver = COUNTER_PER_SECOND;
static u8 counter_for_timeout = TIME_OUT_TASK_INTERVAL;
static u8 counter_for_driver = DRIVER_TASK_INTERVAL;

void reset_all_counters() {
    reset_counter_for_second_time_out();
    reset_counter_for_second_driver();
    reset_counter_for_timeout();
    reset_counter_for_driver();
}


void reset_counter_for_ble() {
    counter_for_second = 200; 
    counter_for_reset = 60;
}

void init_system_tick() {
    Timer0_12T();
    Timer0_AsTimer();
    Timer0_16bitAutoReload();
    T0_Load(0xD8F0);
    Timer0_InterruptEnable();
    Timer0_Run();
}

void sync_with_alert() // 2ms @24.000MHz
{
    unsigned char i, j;
    i = 47;
    j = 174;
    do
    {
        while (--j);
    } while (--i);
}

// 
void tm0_isr() interrupt TIMER0_VECTOR  using 1
{    

    if(!--counter_for_second)
    {
        counter_for_second = 200;
        if(!--counter_for_reset)
        {
            counter_for_reset = 60;
            BC = 0; 
        }
    }


    if (candidate_time_out_signal_recieving)
    {
        --counter_for_second_time_out;
        if (!counter_for_second_time_out)
        {
            reset_counter_for_second_time_out();
            --counter_for_timeout;
            if(!counter_for_timeout)
            {
                INT_PROC |= TIME_OUT_DUTY;
                reset_counter_for_timeout();
            }
        }
    }

    if(candidate_driver_signal_recieving)
    {
        --counter_for_second_driver;
        if(!counter_for_second_driver)
        {
            reset_counter_for_second_driver();
            --counter_for_driver;
            if(!counter_for_driver)
            {
                INT_PROC |= DRIVER_DUTY;
                reset_counter_for_driver();
            } 
        }
    }
}
