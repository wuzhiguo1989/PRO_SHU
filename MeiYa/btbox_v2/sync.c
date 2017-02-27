#include "sync.h"
#include "task.h"
#include "time.h"
#include "share.h"
#include "74hc595.h"
#include "utils.h"
#include "key.h"
#include "adc.h"


// KEY scan 20ms
#define KEY_SCAN_TASK_INTERVAL   10
static u8 key_task_interval = KEY_SCAN_TASK_INTERVAL;

#define BATTERY_CAL_TASK_INTERVAL 100 
static u8 battery_cal_interval = BATTERY_CAL_TASK_INTERVAL;

#define SECOND_SCAN_TASK_INTERVAL 200
static u8 second_task_interval = SECOND_SCAN_TASK_INTERVAL;

#define COUNTER_PER_SECOND 200

#define INTERRUPT_SCAN_TASK_INTERVAL 20
u8 interval_detect_start_signal = INTERRUPT_SCAN_TASK_INTERVAL;
u8 interval_detect_end_signal = INTERRUPT_SCAN_TASK_INTERVAL;

#define INTERRUPT_IGNORE_INTERVAL 200
u16 interval_ignore_end_interrupt = INTERRUPT_IGNORE_INTERVAL;
u16 interval_ignore_start_interrupt = INTERRUPT_IGNORE_INTERVAL;

#define INTERRUPT_START_ON_DELAY 10

#define INTERVAL_WORK_START 600
u16  interval_work_start_on = 0;

void reset_work_start()
{
    interval_work_start_on = INTERVAL_WORK_START;
}

// 5ms
void init_system_tick() {
    Timer0_12T();
    Timer0_AsTimer();
    Timer0_16bitAutoReload();
    T0_Load(0xD8F0);
    Timer0_InterruptEnable();
    Timer0_Run();
}

u16 pca_next_target = 0;
// 2ms 
void dispaly_driver_init() { 
    CCON = 0;                       //初始化PCA控制寄存器 
                                    //PCA定时器停止 
                                     //清除CF标志 
                                    //清除模块中断标志 
     CL = 0; 
     CH = 0; 
     pca_next_target =  pca_next_target + T500Hz; 
     CCAP0L = pca_next_target; 
     CCAP0H = pca_next_target >> 8; 
     CMOD = 0x00;                    //设置PCA时钟源 
                                    //禁止PCA定时器溢出中断 
     CCAPM0 = 0x49;                  //PCA模块0为16位定时器模式 
     CR = 1; 
 } 

#define T1MHz (65536 - MAIN_Fosc / 2 / 12 / 1000000)
void Source_clock_gen(void)
{
    AUXR &= ~0x04;
    AUXR &= ~0x08;
    T2L = T1MHz;
    T2H = T1MHz >> 8;
    INT_CLKO = 0x04;
    Source_Clock_Stop();
    // Source_Clock_Run();
}

void interrupt_control_init()
{
    INT0 = 1;
    IT0 = 0; // 低电平有效
    EX0 = ENABLE;

    INT1 = 1;
    IT1 = 1; // 低电平有效
    EX1 = DISABLE;
}


void tm0_isr() interrupt TIMER0_VECTOR  using 1
{    
    INT_PROC |= TICK_DUTY;
    // 20ms刷新按键
    if(!--key_task_interval)
    {
        INT_PROC |= KEY_DUTY;
        key_task_interval = KEY_SCAN_TASK_INTERVAL;
    }
    // 
    if(!--battery_cal_interval)
    {
        INT_PROC |= ADC_DUTY;
        if(!CHG)
        {
            icon_power ++;
        }    
        else
        {
            if(status_mb_key != 2)
            {
                color_level ++;
                if(color_level == 7)
                    color_level = 0;
            }
        }
        battery_cal_interval = BATTERY_CAL_TASK_INTERVAL;
    } 
}


void count_down_task()
{
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
                    countdown_signal_start = NO;

                    POUT = 0;

                    status_system = 0;

                    status_mb_key = 0;

                    // 关闭+、-灯
                    P23 = 0;
                    //
                    P31 = 1;
                    P12 = 0;
                    P55 = 1;
                    key_sampling = NO;
                    EX0 = ENABLE;
                    EX1 = DISABLE;
                }
                
            }
            second_task_interval = SECOND_SCAN_TASK_INTERVAL;
        }
    }
}

void delay_hold_interval() {
    if(interval_detect_start_signal)
    {
        --interval_detect_start_signal;
        if(interval_detect_start_signal == 1 && status_system == 3)
        {
            status_system = 0;
            P23 = 0;
            EX1 = DISABLE;
        }
        if(interval_detect_start_signal == INTERRUPT_START_ON_DELAY)
            EX1 = ENABLE;

    }

    if (interval_ignore_end_interrupt)
    {
        if(!--interval_ignore_end_interrupt)
        {
            EX1 = ENABLE;
        }
    }


    if (interval_ignore_start_interrupt)
    {
        if(!--interval_ignore_start_interrupt && status_system == 2)
        {
            EX0 = ENABLE;
            status_system = 0;
        }
    }

    if(interval_detect_end_signal)
    {
        --interval_detect_end_signal;
        if(!interval_detect_end_signal && status_system == 4)
        {
            status_system = 1;
            EX0 = DISABLE;
        }
        if(interval_detect_end_signal == INTERRUPT_START_ON_DELAY)
            EX0 = ENABLE;

    }

    if(interval_work_start_on)
    {
        -- interval_work_start_on;
        if(!interval_work_start_on)
        {
            adc_sampling = YES;
            // 美牙蓝牙已启动
            P31 = 0;
            P55 = 1;
            P12 = 1;
            countdown_signal_start = YES;
        }
    }

}


void exint1() interrupt INT1_VECTOR using 3
{
    if(!INT1)
    {
        if(status_system == 3)
        {
            interval_detect_start_signal = 0;
            EX0 = DISABLE;
            EX1 = DISABLE;
            interval_ignore_end_interrupt = INTERRUPT_IGNORE_INTERVAL;
            // 无操作退出
            // no_operation_exit = YES;
            // interval_no_operation = 6000;
            P31 = P12 = P55 = 1;
            // 驱动+、-灯
            P23 = 1;

            status_system = 1;
                
            POUT = 1;

 
            init_adc();
            // 检测按键
            Source_Clock_Run();

            // 显示倒计时
            time_init();

            // 开始无操作关机倒计时
            status_mb_key = 0;

            INT_PROC |= SYSTEM_START_DUTY;
        }
        if(status_system == 1 && status_mb_key != 1)
        {
            if(INT0)
            {
                status_system = 4;
                interval_detect_end_signal = INTERRUPT_SCAN_TASK_INTERVAL;
            }
        }
    }
}

void exint0() interrupt INT0_VECTOR using 1
{
    if(!INT0)
    {
        if(status_system == 0)
        {
            if(!CHG)
            {
                //todo： 直接显示充电模式
                status_system = 5;
                P23 = 0;
                status_mb_key = 0;
            }    
            else
            { 
                if(INT1)
                {
                    status_system = 3;
                    interval_detect_start_signal = INTERRUPT_SCAN_TASK_INTERVAL;
                }
            }
        }

        if(status_system == 4)
        {
            interval_detect_end_signal = 0;
            status_system = 2;
            EX0 = DISABLE;
            EX1 = DISABLE;
            P23 = 0;
            POUT = 0;
            P55 = P31 = P12 = 1;
            interval_ignore_start_interrupt = INTERRUPT_IGNORE_INTERVAL;
            adc_sampling = NO;
            key_sampling = NO;
        }
    }
    else
    {
        // 充电被拔掉或者充电满
        if(status_system == 5)
        {
            status_system = 0;
            Displayclean();

            charge_on = 0;
            countdown_signal_start = NO;
            status_mb_key = 0;
            // 关闭+、-灯
            P23 = 0;
        }

    }
}


#define FAST_CAL_INTERVAL 200
u8 interval_fast_increase_cal = FAST_CAL_INTERVAL;
u8 interval_fast_decrease_cal = FAST_CAL_INTERVAL;
//PCA中断 speak发声 
 void pca_isr() interrupt 7 using 1  
{ 
    CCF0 = 0;       //  清中断标志 
    pca_next_target =  pca_next_target + T500Hz; 
    CCAP0L = pca_next_target; 
    CCAP0H = pca_next_target >> 8;
    INT_PROC |= REFRESH_DUTY;
    if(increase_fast_refresh)
    {
        if(!--interval_fast_increase_cal)
        {
            local_time.minute ++;
            if(local_time.minute == 61)
                local_time.minute = 0;
            interval_fast_increase_cal = FAST_CAL_INTERVAL;
        }
    }
    if(decrease_fast_refresh)
    {
        if(!--interval_fast_decrease_cal)
        {
            if(local_time.minute == 0)
                local_time.minute = 61;
            local_time.minute --;
            interval_fast_increase_cal = FAST_CAL_INTERVAL;
        }
    }
} 
