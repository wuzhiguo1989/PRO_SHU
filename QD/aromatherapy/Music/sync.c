#include "sync.h"
#include "task.h"
#include "config.h"
#include "utils.h"
#include "ble.h"

#define T200Hz  (MAIN_Fosc / 12 / 200)
#define Timer0_Reload   (65536UL - T200Hz)


volatile cycle_control_t xdata led_flicker;  
static u8 key_task_interval = KEY_SCAN_TASK_INTERVAL;


void init_sync_object() {
    led_flicker.cycle_enable = ON;
    led_flicker.counter_for_cycle = 0;
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

void cycle_based_control() {
    // cycle_control_t
    if (led_flicker.cycle_enable) {
        ++led_flicker.counter_for_cycle;
    }

}

// 
void tm0_isr() interrupt TIMER0_VECTOR  using 1
{    
    INT_PROC |= TICK_DUTY;
    // 
    if (!--key_task_interval) {
        INT_PROC |= KEY_DUTY;
        key_task_interval = KEY_SCAN_TASK_INTERVAL;
    }
}
