#include "sync.h"
#include "task.h"
#include "share.h"

#define T200Hz  (MAIN_Fosc / 12 / 200)
#define Timer0_Reload   (65536UL - T200Hz)  

static u8 counter_for_driver = DRIVER_TASK_INTERVAL;

void reset_all_counters() {
    reset_counter_for_driver();
}


void init_system_tick() {
    Timer0_12T();
    Timer0_AsTimer();
    Timer0_16bitAutoReload();
    T0_Load(0xD8F0);
    Timer0_InterruptEnable();
    Timer0_Run();
}

// 
void tm0_isr() interrupt TIMER0_VECTOR  using 1
{    
    if(candidate_driver_signal_recieving)
    {
        --counter_for_driver;
        if(!counter_for_driver)
        {
            reset_counter_for_driver();
            INT_PROC |= DRIVER_DUTY; 
        }
    }
}
