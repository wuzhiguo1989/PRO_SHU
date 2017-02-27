#include "sync.h"
#include "task.h"
#include "share.h"
#include "jl17467pqw.h"
#include "control.h"
#include "beep.h"
#include "light.h"
 
u8 lcd_task_interval = LCD_DISPLAY_TASK_INTERVAL;
u8 second_task_interval = COUNTER_PER_SECOND;
u8 speed_cal_interval = SPEED_TASK_INTERVAL;
u8 counter_per_second_for_time = 2;
u16 int0_counter = 0;

void reset_speed_cal()
{
    int0_counter = 0;
    // flag = 1;
}


void reset_speed_time()
{
    speed_cal_interval = SPEED_TASK_INTERVAL;
}


void init_system_tick() {
    Timer0_12T();
    Timer0_AsTimer();
    Timer0_16bitAutoReload();
    T0_Load(0xB1E0);
    Timer0_InterruptEnable();
    Timer0_Run();
}


void init_speed_driver() {
    INT0 = 1;
    IT0 = 1;                    //  设置INT0的中断类型 (1:仅下降沿 0:上升沿和下降沿)
    EX0 = ENABLE;                
}



// 10ms 定时中断
void tm0_isr() interrupt TIMER0_VECTOR  using 1
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
            speed_counter = int0_counter;
            int0_counter = 0;
            speed_cal_interval = SPEED_TASK_INTERVAL;
        }

    
 
    }



//控制信号中断(上升沿和下降沿中断)
//中断0
void exint0() interrupt INT0_VECTOR  using 3 //INT1中断入口
{
        int0_counter ++;

}

