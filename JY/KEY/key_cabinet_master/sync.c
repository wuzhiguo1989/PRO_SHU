#include "sync.h"
#include "task.h"
#include "share.h"
#include "alarm.h"
#include "config.h"
#include "utils.h"
#include "ble.h"

#define T200Hz  (MAIN_Fosc / 12 / 200)
#define Timer0_Reload   (65536UL - T200Hz)  

static u8 candidate_reset_signal_recieving = NO;
static u8 counter_for_second = 200;
static u8 counter_for_reset = 90;
static u8 counter_for_second_time_out = COUNTER_PER_SECOND;
static u8 counter_for_second_driver = 60;
static u8 counter_for_timeout = TIME_OUT_TASK_INTERVAL;
static u8 counter_for_driver = DRIVER_TASK_INTERVAL;
static u8 key_task_interval = KEY_SCAN_TASK_INTERVAL;
static u8 vac_alarm_counter = 100;
static u16 bat_alarm_counter = 600;
static u8 counter_for_alarm_bat = 0;

void reset_all_counters() {
    reset_counter_for_second_time_out();
    reset_counter_for_second_driver();
    reset_counter_for_timeout();
    reset_counter_for_driver();
}

void reset_counter_for_ble() {
    counter_for_second = 200; 
    counter_for_reset = 90;
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
    if(!candidate_reset_signal_recieving)
    {
        MCU_CS = 1;
        if(!--counter_for_second)
        {
            counter_for_second = 200;
            if(!--counter_for_reset)
            {
                counter_for_reset = 90;
                candidate_reset_signal_recieving = YES;
                init_uart(); // 清除串口数据
                MCU_CS = 0; 
            }
        }
    }
    else
        candidate_reset_signal_recieving = NO;




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
            INT_PROC |= DRIVER_DUTY; 
        }
    }

    if (!--key_task_interval) {
        INT_PROC |= KEY_DUTY;
        key_task_interval = KEY_SCAN_TASK_INTERVAL;
    }



    if(performance_status == 0x80)
    {
        if(!-- vac_alarm_counter)
        {
            ALARM = ~ALARM;
            vac_alarm_counter = 100;
        }
    }

    if(performance_status == 0x40)
    {
        if(!-- bat_alarm_counter)
        {
            counter_for_alarm_bat ++;
            if(counter_for_alarm_bat == 4)
            {
                ALARM = 0;
                counter_for_alarm_bat = 0;
                bat_alarm_counter = 400;
            }
            else
            {
                ALARM = ~ALARM;
                bat_alarm_counter = 100;
            }
            
        }
    }
}
