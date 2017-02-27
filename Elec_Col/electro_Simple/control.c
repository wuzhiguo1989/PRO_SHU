#include "control.h"
#include "string.h"
#include "task.h"
#include "stdio.h"
#include "jl17467pqw.h"
#include "e2prom.h"
#include "share.h"
#include "sync.h"

#define circumference  957L 

static volatile u16  xdata mileage_per_second = 0;
static volatile u32  xdata mileage_in_total;
static volatile u32  xdata mileage_in_total_simple;
static volatile u32  xdata mileage_store;
static volatile u32  xdata mileage = 0;
u16  period_speed_counter[SPEED_CAL_COUNTER];
u8  period_interval = 0;
u16 speed_counter_total = 0;
// u16 speed_counter_total_pre = 0;

void Store_mileage_in_total() 
{
    if(local == 511)
    {
        local = 0;
        mileage_in_total_simple ++;
        Store_mileage_total(mileage_in_total_simple);
    }
    else
    {
        Store_mileage(local);
    }
    local ++;
}



void reset_speed_counter()
{
    u8 i;
    for(i = 0; i < SPEED_CAL_COUNTER; i++)
        period_speed_counter[i] = 0;
    period_interval = 0;
    speed_counter_total = 0;
}


void control_init() {
    local = Read_mileage();
    mileage_in_total_simple = Read_mileage_total();
    mileage_in_total = (mileage_in_total_simple * 512 + local) * 10000; // cm
    mileage_store = mileage_in_total;
    jt17467pqw_set_speed(mileage_per_second);
    jt17467pqw_set_mileage(mileage);
    jt17467pqw_set_mileage_total(mileage_in_total/10000);
 //   jt17467pqw_set_power(power_per_second);
}


void speed_cal_display()
{
    u8 i;
    u16 speed;
    if(speed_counter < 2)
    {
        speed = 0;
        reset_speed_counter();
    }
    else
    {
        period_speed_counter[period_interval++] = speed_counter;
        if(period_interval == SPEED_CAL_COUNTER)
            period_interval = 0;

        speed_counter_total = 0;

        for(i = 0; i < SPEED_CAL_COUNTER; i++)
            speed_counter_total = speed_counter_total + period_speed_counter[i];
        speed_counter_total >>= 3;
        // 36V
        speed = speed_counter_total * 62 / 10; //cm
        // 48V
        //speed = speed_counter_total * 63 / 10; //cm
    }

    mileage_in_total +=  speed;
    mileage += speed;
    // 36V
    speed =  speed * 72 / 10;
    // 48V
    //speed =  speed * 8;
    jt17467pqw_set_speed(speed / 10);
    jt17467pqw_set_mileage(mileage / 10000);
    jt17467pqw_set_mileage_total(mileage_in_total / 10000);
    // 保存公里数 
    if(mileage_in_total / 10000 - mileage_store / 10000 >= 1)
    {
        reset_speed_cal();
        reset_speed_time();
        Store_mileage_in_total();
        mileage_store = mileage_in_total;
    }
}



