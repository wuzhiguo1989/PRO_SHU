#include "sync.h"
#include "task.h"
#include "config.h"
#include "utils.h"
#include "power.h"
#include "lm567.h"

#define T200Hz  (MAIN_Fosc / 12 / 200)
#define Timer0_Reload   (65536UL - T200Hz)


typedef struct time_entity
{
    u8 minute;      /* minutes, range 0 to 59           */
    u8 second;      /* seconds, range 0 to 59           */
} time_t;

time_t local_time;

volatile bit countdown_signal_start = NO;
volatile cycle_control_t xdata led_flicker;  
static u8 key_task_interval = KEY_SCAN_TASK_INTERVAL;
#define SECOND_SCAN_TASK_INTERVAL 200
static u8 second_task_interval = SECOND_SCAN_TASK_INTERVAL;


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

u8 minute_count_down[5] = {0, 15, 30, 60, 90};

void countdown_off(u8 time_index)
{
    u8 minute = minute_count_down[time_index];
    if(!minute)
        countdown_signal_start = NO;
    else
    {
        countdown_signal_start = YES;
        second_task_interval = SECOND_SCAN_TASK_INTERVAL;
        local_time.minute = minute;
        local_time.second = 0;
    }
}

// 
void tm0_isr() interrupt TIMER0_VECTOR  using 1
{    
    INT_PROC |= TICK_DUTY;
    if (!--key_task_interval) {
        INT_PROC |= KEY_DUTY;
        key_task_interval = KEY_SCAN_TASK_INTERVAL;
    }
    // 1000ms 刷新一次时间
    if(countdown_signal_start)
    {
        if(!--second_task_interval)
        {
            if(--local_time.second == 255)
            {
                local_time.second = 59;
                if(!local_time.minute --)
                {
                    countdown_signal_start = NO;
                    INT_PROC |= SLEEP_DUTY;
                }
                
            }
            second_task_interval = SECOND_SCAN_TASK_INTERVAL;
        }
    }
}
