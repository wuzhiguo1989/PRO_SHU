#include "sync.h"
#include "task.h"
#include "utils.h"
#include "power.h"
#include "lm567.h"

u8 idata key_task_interval = KEY_SCAN_TASK_INTERVAL;

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
    INT_PROC |= TICK_DUTY;
    if (!--key_task_interval) {
        INT_PROC |= KEY_DUTY;
        key_task_interval = KEY_SCAN_TASK_INTERVAL;
    }
}
