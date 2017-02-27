#include "control.h"
#include "string.h"
#include "usart.h"
#include "task.h"
#include "stdio.h"
#include "jl17467pqw.h"
#include "e2prom.h"
#include "share.h"

#define circumference  957L 


static volatile u8 xdata RX2_Buffer[COM_RX2_Lenth];    //接收缓冲
static volatile u16  xdata mileage_per_second = 0;
static volatile u32  xdata mileage_in_total;
static volatile u32  xdata power_per_second = 0;
static volatile u32  xdata mileage = 0;
u16  period_speed_counter[SPEED_CAL_COUNTER];
u8  period_interval = 0;
u16 speed_counter_total = 0;
// u16 speed_counter_total_pre = 0;

void Store_mileage() 
{
    Store_mileage_total(mileage_in_total);
}



void reset_speed_counter()
{
    u8 i;
    for(i = 0; i < SPEED_CAL_COUNTER; i++)
        period_speed_counter[i] = 0;
    period_interval = 0;
    speed_counter_total = 0;
}

void mileage_init()
{
    mileage = 0;
}

void control_init() {
    
    mileage_in_total = Read_mileage_total(); // cm

    jt17467pqw_set_speed(mileage_per_second);
    jt17467pqw_set_mileage(mileage);
    jt17467pqw_set_mileage_total(mileage_in_total/10000);
    jt17467pqw_set_power(power_per_second);
    memset(RX2_Buffer, 0, COM_RX2_Lenth);
    USART_Init(&COM2); 
// 只能使用定时器2做波特率发生器
    S2_8bit();
    S2_USE_P10P11();

    S2_RX_Enable();
    S2_Int_Enable();    
    USART_Share_Timer2();
    SET_TI2();
}


void speed_cal_display()
{
    u8 i;
    u16 speed;
    if(!speed_counter)
    {
        speed = 0;
        reset_speed_counter();
    }
    else
    {
        speed = (circumference * 360) / speed_counter; //0.01km/h
        mileage_in_total += circumference / 10; // cm 
        mileage += circumference / 10;
    }
    
    period_speed_counter[period_interval++] = speed;
    if(period_interval == SPEED_CAL_COUNTER)
            period_interval = 0;

    speed_counter_total = 0;

    for(i = 0; i < SPEED_CAL_COUNTER; i++)
        speed_counter_total = speed_counter_total + period_speed_counter[i];

    speed_counter_total >>= 3;

    jt17467pqw_set_speed(speed_counter_total / 10);

    // speed_counter_total_pre = speed_counter_total;


    jt17467pqw_set_mileage(mileage / 10000);
    jt17467pqw_set_mileage_total(mileage_in_total / 10000); 
}


// 直接赋值
void control_process_received() {
    power_per_second = (RX2_Buffer[1] << 8 | RX2_Buffer[2]);
    jt17467pqw_set_power(power_per_second);
}

// 保证每次取到值
void UART2_int (void) interrupt 8
{
    if((S2CON & 1) != 0)
    {
        if(S2BUF == 'S' && !COM2.B_RX_OK)
            COM2.B_RX_OK = 1;
        if(COM2.B_RX_OK)
            RX2_Buffer[COM2.RX_Cnt++] = S2BUF;
        if (COM2.RX_Cnt == COM_RX2_Lenth)    
        {
            COM2.RX_Cnt = 0;
            COM2.B_RX_OK = 0;
            INT_PROC |= CONTROL_RECEIVE_DUTY;
        } 
        S2CON &= ~1;    //Clear Rx flag
    }

    if((S2CON & 2) != 0)
    {
        S2CON &= ~2;    //Clear Tx flag
        COM2.B_TX_busy = 0;
    }
}

