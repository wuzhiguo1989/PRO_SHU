#include "sync.h"
#include "display.h"
#include "shared.h"
#include "intrins.h"
#include "stc15w408as.h"
#include "def.h"
#include "led.h"

static u16 counter_for_pca5ms = PCA5ms;
static u8 sync_5ms = 0;
static u32 time_counter_store = 0;
static bit interrupt_duty = 0;

void init_sync_timer() {// 定时器0 0.01ms
    AUXR &= 0x7F;       //定时器时钟12T模式
    TMOD &= 0xF0;       //设置定时器模式
    TL0 = 0xF1;     //设置定时初值
    TH0 = 0xFF;     //设置定时初值
    TF0 = 0;        //清除TF0标志
    TR0 = 1;        //定时器0开始计时
    PT0 = 1;
    ET0 = 1;
}

void init_ac_driver() {
//    PX0 = 1;
    INT0 = 1;
    IT0 = 0;                    //  设置INT0的中断类型 (1:仅下降沿 0:上升沿和下降沿)
    EX0 = ENABLE;                
}


void reset_all_counters() {
    reset_counter_for_cycle();
}



void exint0_isr() interrupt 0 using 1  
{
    if(!INT0)
        interrupt_duty = 1;
}

// 5ms
void tm0_isr() interrupt 1 using 3       // interval 5ms 
{

    // 5ms old tick-tock 
    --counter_for_pca5ms;
    if (!counter_for_pca5ms) {
        INT_PROC |= TICK_DUTY;
        ++counter_for_cycle;
        counter_for_pca5ms = PCA5ms;
    }
    if(interrupt_duty)
    {
        ++time_counter_store;
        if(INT0)
        {
            interrupt_duty = 0;
            time_counter = time_counter_store;
            time_counter_store = 0;
            if(time_counter >= 60)
                INT_PROC |= EXINT_PROC;
            else
                time_counter = 0;
        }
    }
}