#include "sync.h"
#include "task.h"
#include "share.h"
#include "jl17467pqw.h"
#include "control.h"
#include "beep.h"
#include "signal.h"
#include "light.h"
 
u8 lcd_task_interval = LCD_DISPLAY_TASK_INTERVAL;
u8 second_task_interval = COUNTER_PER_SECOND;
u8 speed_cal_interval = SPEED_TASK_INTERVAL;
u8 counter_per_second_for_time = 2;
u16 counter_for_2s = COUNTER_FOR_SECOND_2S; // count_for_2s的变量是控制震动报警开始的时间的 分为2s 和 10s
u16 counter_for_10s = COUNTER_FOR_SECOND_10S; 
u16 int0_counter = 0;
u8 flag = 0;

void reset_2s_counter()
{
    counter_for_2s = COUNTER_FOR_SECOND_2S;
}

void reset_2s_counter_for_key()
{
    counter_for_2s = COUNTER_FOR_SECOND_10S;
}

void reset_10s_counter()
{ 
    counter_for_10s = COUNTER_FOR_SECOND_10S;
}

void reset_time_counter()
{
    second_task_interval = COUNTER_PER_SECOND;
    counter_per_second_for_time = 2;
}


void init_system_tick() {
    Timer0_12T();
    Timer0_AsTimer();
    Timer0_16bitAutoReload();
    T0_Load(0xB1E0);
    Timer0_InterruptEnable();
    Timer0_Run();
}

void init_system_tick1() {
    Timer1_12T();
    Timer1_AsTimer();
    Timer1_16bitAutoReload();
    T1_Load(0xF830);
    Timer1_InterruptEnable();
    // Timer1_Stop();
    Timer1_InterruptFirst();
    Timer1_Run();
}

void init_speed_driver() {
    INT0 = 1;
    IT0 = 1;                    //  设置INT0的中断类型 (1:仅下降沿 0:上升沿和下降沿)
    EX0 = ENABLE;                
}


// 震动报警中断
// 下降沿有效
void exint2() interrupt 10          //INT2中断入口
{   
//    INT_CLKO &= 0xEF;
    if(!speak_mode_set)
    {
        state_enter_trans(SHOCK_SIGNAL);
        // 震动以后切换报警时间
        first_alert = 1;
    }                           
}


// 10ms 定时中断
void tm0_isr() interrupt TIMER0_VECTOR  using 1
{    
    if(state_current == NORMAL_STATE || state_current == KEY_NORMAL_STATE)
    {
        // 10ms 灯
        INT_PROC |= KEY_SCAN_DUTY;

        // 500ms 刷新一次时间
        if(!--second_task_interval)
        {
            INT_PROC |= TIME_SYNC_DUTY;
            // 计时图标闪烁
            time_colon_icon = ~time_colon_icon;

            // 计算时间
            if(!--counter_per_second_for_time)
            {
                if(local_time.second++ == 59)
                {
                    local_time.second = 0;
                    if(local_time.minute++ == 59)
                    {
                        local_time.minute = 0;
                        local_time.hour ++;
                    }
                }
                counter_per_second_for_time = 2;
            }
            second_task_interval = COUNTER_PER_SECOND;
        }

        // 20ms刷新LCD
        if (!--lcd_task_interval) {
            INT_PROC |= LCD_DISPLAY_DUTY;
            lcd_task_interval = LCD_DISPLAY_TASK_INTERVAL; 
        }


        //  500ms获取一次速度、刷新一次温度和电压
        if(!--speed_cal_interval)
        {
            INT_PROC |= ADC_DUTY;
            speed_cal_interval = SPEED_TASK_INTERVAL;
        }
    }

    
    // 首次震动10s 计时
    if(enter_lock_state_15s)
    {
        if(!--counter_for_10s)
        {
            // 震动报警模式变换
            reset_10s_counter();
            enter_lock_state_15s = NO;
            first_alert = 0;
        }

    }

    //铃声响一次1s内不再响应
    if(enter_lock_state_2s)
    {
        if(!--counter_for_2s)
        {
            INT_CLKO |= 0x10;
            reset_2s_counter();
            enter_lock_state_2s = NO;
        }
    }

        // 断续发声(四个按键的时序声音)
    if(speak_mode_set & 0x3f)
    {
        if(!--speak_time)
        {    
            // 解锁键声音   
            if(speak_mode_set == 0x01){
                MUTE = ~MUTE;
                interval ++;
                // 换频率
                if(interval == 4)
                    speak_mode = unlockmode;
                speak_time = speak_time_interval[speak_mode - 1];
                // 一个周期到
                if(interval == 5)
                {
                    speak_mode_set &= ~0x003f;
                    state_current = NORMAL_STATE;
                }
            }
            // 4s报警 
            else if(speak_mode_set == 0x08){
                if(speak_mode == firstalertmode)
                    speak_mode = secondalertmode;
                else
                    speak_mode = firstalertmode;
                interval ++;
                speak_time = speak_time_interval[speak_mode - 1];
                // 一个周期到
                if(interval == 50)
                {
                    MUTE = ~MUTE;
                    state_current = LOCK_STATE;
                    reset_10s_counter();
                    enter_lock_state_15s = YES;
                    speak_mode_set &= ~0x003f;
                    wait_for_alert();
                }
            }

            // 15秒报警 
            else if(speak_mode_set == 0x10) {
                if(speak_mode == firstalertmode)
                    speak_mode = secondalertmode;
                else
                    speak_mode = firstalertmode;
                interval ++;
                // 一个周期到
                speak_time = speak_time_interval[speak_mode - 1];
                if(interval == 200)
                {
                    MUTE = ~MUTE;
                    state_current = LOCK_STATE;
                    reset_10s_counter();
                    enter_lock_state_15s = YES;
                    speak_mode_set &= ~0x003f;
                    wait_for_alert();
                }
            }

            // 锁定 
            else if (speak_mode_set == 0x04){
                if(speak_mode == firstalertmode)
                    speak_mode = secondalertmode;
                else
                    speak_mode = firstalertmode;
                interval ++;
                // 一个周期到
                speak_time = speak_time_interval[speak_mode - 1];
                if(interval == 6)
                {
                    MUTE = ~MUTE;
                    speak_mode_set &= ~0x003f;
                    wait_for_alert();
                }
            }

            else  
            {
                speak_time = speak_time_interval[speak_mode - 1];
                MUTE = ~MUTE;
                speak_mode_set &= ~0x003f;
                wait_for_alert();
            }

            light_follow_speak();
        }
    }

}

void tm1_isr() interrupt TIMER1_VECTOR  using 2
{
    int0_counter ++;
    if(int0_counter == 1000)
    {
        speed_counter = 0;
        int0_counter = 0;
        flag = 1;
        INT_PROC |= SPEED_CAL_DUTY;
    }
}

//控制信号中断(上升沿和下降沿中断)
//中断0
void exint0() interrupt INT0_VECTOR  using 3 //INT1中断入口
{
    if(state_current == NORMAL_STATE || state_current == KEY_NORMAL_STATE)
    { 
        speed_counter = int0_counter;
        int0_counter = 0;
        if(!flag)
            INT_PROC |= SPEED_CAL_DUTY;
        else
            flag = 0;
    }
}

