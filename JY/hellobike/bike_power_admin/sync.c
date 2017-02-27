#include "sync.h"
#include "task.h"
#include "utils.h"
#include "power.h"  

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

// cal_timer
// 20ms
void init_system_tick() {
    Timer0_12T();
    Timer0_AsTimer();
    Timer0_16bitAutoReload();
    T0_Load(0xD8F0);
    Timer0_InterruptEnable();
    Timer0_Run();
}


// driver_timer
void init_driver_timer() {
    Timer2_Stop();      //Timer stop
    Timer2_AsTimer();   //Timer2 set As Timer
    Timer2_12T() ;       //Timer2 set as 1T mode
    T2L = 0xEC;         //设定定时初值
    T2H = 0xFF;         //设定定时初值
    Timer2_InterruptEnable(); //禁止中断
    // Timer2_Run();      //Timer stop
}

// 25 = 500ms
u8  fre_cal_interval = FRE_CAL_TASK_INTERVAL;
// 
void tm0_isr() interrupt TIMER0_VECTOR  using 2
{    
    if (!--fre_cal_interval) {
        INT_PROC |= FRE_CAL_DUTY;
        fre_cal_interval = FRE_CAL_TASK_INTERVAL;
        // CHAR_DRI = ~CHAR_DRI;
    }

}


void tm2_isr() interrupt TIMER2_VECTOR using 0
{
    // 充电有效进中断、载入无效时间
    if(CHAR_DRI){
        driver_timer_disable();
    // 充电无效进中断、载入有效时间
    } else {
        driver_timer_enable();
    }
    // CHAR_DRI = ~CHAR_DRI;
}
