#include "sync.h"
#include "task.h"
#include "time.h"
#include "share.h"
#include "74hc595.h"
#include "utils.h"
#include "key.h"

u8 second_task_interval = COUNTER_PER_SECOND;
u8 key_task_interval = 4;
u8 speed_cal_interval = SPEED_TASK_INTERVAL;
u16 countdown_5s_interval = COUNTDOWN;
u8 second_speak_interval = KEY_SCAN_TASK_INTERVAL;
u8 speak_timer = 0;

void reset_countdown_5s()  
{
    countdown_5s_interval = COUNTDOWN;
}

void reset_countdown_speak()  
{
    second_speak_interval = KEY_SCAN_TASK_INTERVAL;
}

void init_system_tick() {
    Timer0_12T();
    Timer0_AsTimer();
    Timer0_16bitAutoReload();
    T0_Load(0xD8F0);
    Timer0_InterruptEnable();
    Timer0_Run();
}


void Timer2Init(void)       //100微秒@24.000MHz
{
    Timer2_12T();
    Timer2_AsTimer();
    T2_Load(0xE890);
    Timer2_InterruptEnable();
    Timer2_Run();
}



void tm0_isr() interrupt TIMER0_VECTOR  using 1
{    

    if(!--key_task_interval)
    {
        INT_PROC |= KEY_DUTY;
        key_task_interval = 4;
    }
    //
    if(!--speed_cal_interval)
    {
        INT_PROC |= ADC_DUTY;
        speed_cal_interval = SPEED_TASK_INTERVAL;
    }

    // 1000ms 刷新一次时间
    if(countdown_signal_start)
    {
        if(!--second_task_interval)
        {
            // 计时图标闪烁
            time_colon_icon = ~time_colon_icon;
            // 计算时间
            if(--local_time.second == 255)
            {
                local_time.second = 59;
                if(!local_time.minute --)
                {
                    POUT = 0;
                    status_mb_key = 0;
                    countdown_signal_start = NO;
                    speak_timer = 0;
                    mute_beep_enable();
                    speak_mode_set |= 0x02;
                }
                

            }
            second_task_interval = COUNTER_PER_SECOND;
        }
    }

    if(enter_5s_countdown)
    {
        if(!--countdown_5s_interval)
        {
            status_mb_key = 0;
            enter_5s_countdown = NO;
        }
    }

    if(speak_mode_set & 0x01)
    {
        if(!--second_speak_interval)
        {
            second_speak_interval = KEY_SCAN_TASK_INTERVAL;
            speak_mode_set &= ~0x01;
        }
    }

    if(speak_mode_set & 0x02)
    {
        if(!--second_speak_interval)
        {
            second_speak_interval = KEY_SCAN_TASK_INTERVAL;
            speak_timer ++;
            MUTE = ~ MUTE;
            if(speak_timer == 3)
            {
                speak_mode_set &= ~0x02;
                speak_timer = 0;
            }
        }
    }

}

void tm2_isr() interrupt TIMER2_VECTOR  using 2
{
    INT_PROC |= REFRESH_DUTY;
}


