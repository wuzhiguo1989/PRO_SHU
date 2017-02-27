#include "task.h"
#include <string.h>
#include "adc.h"
#include "alert.h"
#include "task.h"
#include "config.h"
#include "uart.h"
#include "process.h"
#include "e2prom.h"
#include <stdio.h>
#include <stdlib.h>

// SYSTEM ALL INT MASK
volatile u16 xdata INT_PROC = 0;
// 系统开始停止状态位
volatile bool start_fan_signal = 0;
// 放料过程料位时间计时标记位
volatile bool signal_detect_enable = NO;
// 延时开机计时状态位
volatile bool signal_feed_full_delay = NO;
// 放料确认完成
volatile bool signal_feed_lack_keep = NO;

bool signal_rise_fall = 0;

u8 counter_for_spray_wash_logic;
u8 lack_pre = 0;


#define T200Hz  (MAIN_Fosc / 12 / 100)
#define T0_THL  (65536ul - T200Hz)
#define COUNTER_PER_SECOND      100
#define SECONDS_PER_MINUTE      60


// 缺料确认时间
static xdata u16 time_feed_lack_keep = 0;
// 料满延迟启动时间
static xdata u16 time_task_interval = 0;
// 缺料检测
static xdata u16 time_not_exist = 0;
// 记录料满时间
static xdata u16 time_feed_full = 0;

static u8 second_task_interval = COUNTER_PER_SECOND;  

void system_tick_init() {
    Timer0_12T();
    Timer0_AsTimer();
    Timer0_16bitAutoReload();
    Timer0_Load(T0_THL);
    Timer0_InterruptEnable();
    Timer0_Stop();
}

void Extern0_Init(void)
{
    INT0 = 1;
    IT0 = 0;                    //  设置INT0的中断类型 (1:仅下降沿 0:上升沿和下降沿)
    EX0 = 1;                    //  使能INT0中断
}

void reload_feed_suction(u8 local_nmber)
{ 
    count_down_reload(&feed_suction[local_nmber], time_feed_suction_set[local_nmber], 0);
}

void reload_feed_discharge(u8 local_nmber)
{ 
    count_down_reload(&feed_discharge[local_nmber], time_feed_discharge_set[local_nmber], 0);
}

void reload_feed_intercept(u8 local_nmber) 
{
    count_down_reload(&feed_intercept[local_nmber], time_feed_intercept_set[local_nmber], 0);
}

void reload_spray_wash(u8 local_nmber) 
{
    count_down_reload(&spray_wash[local_nmber], time_spray_wash_set[local_nmber] / 10, time_spray_wash_set[local_nmber] % 10 * COUNTER_PER_100MS);
}

void reload_spray_wash_between(u8 local_nmber) 
{
    count_down_reload(&spray_wash_between[local_nmber], time_spray_wash_between_set[local_nmber]/ 10, time_spray_wash_between_set[local_nmber] % 10 * COUNTER_PER_100MS);
}

void reload_spray_wash_stop(u8 local_nmber) 
{
    count_down_reload(&spray_wash_stop[local_nmber], 1, 0);
}

// test
void reload_test_spray_wash(u8 local_nmber) 
{
    count_down_reload(&test_spray_wash[local_nmber], 10, 0);
}

void reload_test_suction(u8 local_nmber) 
{
    count_down_reload(&test_suction[local_nmber], 10, 0);
}

void reload_test_intercept(u8 local_nmber) 
{
    count_down_reload(&test_intercept[local_nmber], 10, 0);
}


void system_start_or_not()
{
    // 温度、过载没解除
    if(alert_for_system & 0x14)
        start_fan_signal = 0;
    if(!start_fan_signal)
        INT_PROC |= SYSTEM_END_DUTY;
    else
        INT_PROC |= START_DUTY;
}

void exint0_isr() interrupt 0 using 1 
{
    if(!INT0)
    {
        start_fan_signal = ~start_fan_signal;
        system_start_or_not();
    }
}

void call_for_duty(u16 duty_code) {
	INT_PROC |= duty_code;
}

bool working_for_duty(u16 duty_code) {
    return INT_PROC & duty_code;
}

count_down_t xdata fan_start_deceted;
count_down_t xdata feed_suction[NUMBER_BUNKER];
count_down_t xdata feed_discharge[NUMBER_BUNKER];
count_down_t xdata feed_intercept[NUMBER_BUNKER];
count_down_t xdata spray_wash[NUMBER_BUNKER];
count_down_t xdata spray_wash_between[NUMBER_BUNKER];
count_down_t xdata spray_wash_stop[NUMBER_BUNKER];

count_down_t xdata test_spray_wash[NUMBER_BUNKER];
count_down_t xdata test_suction[NUMBER_BUNKER];
count_down_t xdata test_intercept[NUMBER_BUNKER];
count_down_t xdata test_fan;
count_down_t xdata test_beep;

// system
count_down_t xdata system_back_home;
count_down_t xdata password_no_identified;


void count_down_init(count_down_t xdata *cnt, event_cb func) {
    cnt -> second = 0;
    cnt -> millisecond = 0;
    cnt -> p_callback = func;
}

void count_down_reload(count_down_t xdata *cnt, const u8 second, const u8 millisecond) {
    if(!millisecond) {
        cnt -> second = second - 1;
        cnt -> millisecond = COUNTER_PER_SECOND;
    } else {
        cnt -> second = second;
        cnt -> millisecond = millisecond;
    }
}


bool count_down_at(count_down_t xdata *cnt) {
if (!cnt -> second && !cnt -> millisecond)
        return FALSE;
    
    if (!--cnt -> millisecond)      // --ms == 0
    {
        if (!cnt -> second) {
            if (cnt -> p_callback)
                cnt -> p_callback();
            return TRUE;
        } else {
            --cnt -> second;
            cnt -> millisecond = COUNTER_PER_SECOND;
        }
    }

    return FALSE;
}

void count_down_cancel(count_down_t xdata *cnt) {
    cnt -> second = 0;
    cnt -> millisecond = 0;
    // cnt -> p_callback = NULL;
}



void count_down_all_task() {
    u8 i;
	count_down_at(&fan_start_deceted);
    for(i = 0; i < NUMBER_BUNKER; i++)
    {
        count_down_at(&feed_discharge[i]);
        count_down_at(&feed_suction[i]);
        count_down_at(&feed_intercept[i]);
        count_down_at(&spray_wash[i]);
        count_down_at(&spray_wash_between[i]);
        count_down_at(&spray_wash_stop[i]);
// test
        count_down_at(&test_spray_wash[i]);
        count_down_at(&test_suction[i]);
        count_down_at(&test_intercept[i]);
    }
    count_down_at(&test_fan);
    count_down_at(&test_beep);
// system
    count_down_at(&system_back_home);
    count_down_at(&password_no_identified);
}

void count_down_cancal_bunker(u8 local_nmber)
{
    count_down_cancel(&feed_suction[local_nmber]);
    count_down_cancel(&feed_discharge[local_nmber]);
    count_down_cancel(&feed_intercept[local_nmber]);
    count_down_cancel(&spray_wash[local_nmber]);
    count_down_cancel(&spray_wash_between[local_nmber]);
    count_down_cancel(&spray_wash_stop[local_nmber]);
}

void count_down_cancle_all_task(){
    u8 i;
    count_down_cancel(&fan_start_deceted);
    for(i = 0; i < NUMBER_BUNKER; i++)
        count_down_cancal_bunker(i);
    start_fan_signal = 0;
}


void count_down_cancel_test()
{
    u8 i;
    for(i = 0; i < NUMBER_BUNKER; i++)
    {
        count_down_cancel(&test_spray_wash[i]);
        count_down_cancel(&test_suction[i]);
        count_down_cancel(&test_intercept[i]);
    }
    count_down_cancel(&test_fan);
    count_down_cancel(&test_beep);
}


void spray_wash_start_callback(u8 local_bunker)
{
    printf("spray_wash, PoW\r\n");
    spray_wash_enable(local_bunker);
    reload_spray_wash(local_bunker);
}

// = 0 说明不能启动； >0 启动
u8 fan_signal_detect()
{
    // 判断处于缺料的吸料机
    u8 i, fan_status;
    for(i = 0; i < NUMBER_BUNKER; i++)
    {
        if(!IO_FEED_LACK[i])
        {
            status_for_bunker[i] = feed_lack;
            fan_status |= 1;
            signal_feed_full_delay[i] = NO;
        // TODO : 需要提示报警
        //    alert_for_system &= ~ALTER_FULL_FEED; 
        }
        else
        {
            status_for_bunker[i] = feed_full;
            fan_status &= 0;
            signal_feed_full_delay[i] = YES;
            // TODO : 需要提示报警
            // alert_for_system &= ~ALTER_FULL_FEED; 
            // signal_feed_full_delay = YES;
            // // regard as rise 
            // signal_rise_fall = 1;
            // system_enter_alter_feed_full();
            // alert_for_system &= ~ALTER_LACK_FEED;
            // alert_for_system |= ALTER_FULL_FEED;
        }
        fan_status = << 1;
    }
    return fan_status;
}

void detected_feed_full_start()
{

    if(fan_signal_detect())
    {
        printf("PoW1\r\n");
        reload_fan_start_deceted();
        fan_work_enable();
        system_enter_fan_work();
    }
}

// 开始截料
void feed_intercept_callback(u8 local_bunker)
{
    printf("FEED_INTERCEPT, PoW\r\n");
    intercept_enable(local_bunker);
    system_enter_intercept(local_bunker);
}

// 吸料
void feed_suction_task(u8 local_bunker)
{
    // 吸料
    suction_enable(local_bunker);
    // 开始记录料满时间
    time_feed_full[local_bunker] = 0;
    // 截料时间 >= 吸料时间
    if(!time_feed_intercept_set[local_bunker])
        feed_intercept_callback(local_bunker);
    else 
    {
        printf("FEED_SUCTION, PoW\r\n");
        system_enter_sunction(local_bunker);
    }

    if(time_feed_intercept_set[local_bunker] != time_feed_suction_set[local_bunker])
        reload_feed_intercept(local_bunker);
    reload_feed_suction(local_bunker);
}


// 启动时间到、缺料的开始反吹
void fan_start_deceted_callback()
{
    u8 i;
    if(update_data_should)
    {
        // 所有的值改变一下
        updata_data_from_dwscreen();
        update_data_should = false;
    }
    for(i = 0; i < NUMBER_BUNKER; i++)
    {
        if(status_for_bunker[i] == feed_lack)
        {
            counter_for_spray_wash_logic[i] = counter_for_spray_wash_set[i];
            if(counter_for_spray_wash_logic[i])
                spray_wash_start_callback(i);
            else
                feed_suction_task(i);
        }
    }
}





// 吸料时间到、进放料
void feed_suction_callback(u8 local_bunker)
{
    printf("FEED_DISCHARGE\r\n");
    intercept_disable(local_bunker);
    //放料
    discharge_enable(local_bunker);
    //放料动画
    system_enter_discharge(local_bunker);
    reload_feed_discharge(local_bunker);
    //检测是否料满过
    // 没有料满
    if(feed_lack_signal_set[local_bunker])
    {
        signal_detect_enable[local_bunker] = YES;
        time_not_exist[local_bunker] = 0;
    }
    else
    {
        signal_detect_enable[local_bunker] = NO;
        time_not_exist[local_bunker] = 0;
    }
}




void spray_wash_callback(u8 local_bunker)
{
    //关
    printf("dengdai\r\n");
    spray_wash_disable(local_bunker);
    counter_for_spray_wash_logic[local_bunker] --;
    if(!counter_for_spray_wash_logic[local_bunker])
        reload_spray_wash_stop(local_bunker);
    else
        reload_spray_wash_between(local_bunker);
}

void spray_wash_between_callback(local_bunker)
{
    spray_wash_enable(local_bunker);
    reload_spray_wash(local_bunker);
}

void spray_wash_stop_callback(u8 local_bunker)
{
    // 等待确认
    printf("guangji\r\n");
    feed_suction_task(local_bunker);
}

void start_spray_wash(u8 local_bunker)
{
    if(update_data_should)
    {
        // 所有的值改变一下
        updata_data_from_dwscreen();
        update_data_should = false;
    }
    counter_for_spray_wash_logic[local_bunker] = counter_for_spray_wash_set[local_bunker];
    if(counter_for_spray_wash_set)
        // reload_spray_wash_start();
        spray_wash_start_callback(local_bunker);
    else
        feed_suction_task(local_bunker);
}


// 放料时间到
void feed_discharge_callback(u8 local_bunker)
{
    printf("g\r\n");
    if(feed_lack_signal_set[local_bunker])
    {
        if(time_not_exist[local_bunker] < time_feed_lack_set[local_bunker] * COUNTER_PER_100MS)
        // 缺料计数+1
        {
            counter_for_feed_lack[local_bunker] ++;
            if(counter_for_feed_lack[local_bunker] >= counter_for_lack_alter_set[local_bunker])
             // todo
                alert_for_system |= ALTER_LACK_FEED;
            else
            // todo
                alert_for_system &= ~ALTER_LACK_FEED;
            // todo
            send_counter_for_feed_lack_to_dwscreen(counter_for_feed_lack[local_bunker], local_bunker);
        }
        else
        {
            // todo 
            alert_for_system &= ~ALTER_LACK_FEED;
            if(counter_for_feed_lack[local_bunker])
            {
                counter_for_feed_lack[local_bunker] = 0;
                send_counter_for_feed_lack_to_dwscreen(counter_for_feed_lack[local_bunker], local_bunker);
            }
        }
    }
    else
    {
        if(!counter_for_feed_lack[local_bunker])
        {
            counter_for_feed_lack[local_bunker] = 0;
            send_counter_for_feed_lack_to_dwscreen(counter_for_feed_lack[local_bunker], local_bunker);
        }
        // todo
        alert_for_system &= ~ALTER_LACK_FEED;
    }

    time_not_exist[local_bunker] = 0;
    signal_detect_enable[local_bunker] = NO;
    // 说明没有关闭
    // 依旧料满
    if(IO_FEED_LACK[local_bunker])
    {
        // todo
        feed_status = wait_for_discharge_end;
    }
    // 无料
    else
    // 反吹
        start_spray_wash(local_bunker);
}


// test callback
void test_spray_wash_callback()
{
    test_reset_para(ADDR_TEST_SPRAY_WASH);
    spray_wash_disable();
}

void test_suction_callback()
{
    test_reset_para(ADDR_TEST_SUCTION);
    discharge_enable();
}

void test_intercept_callback()
{
    test_reset_para(ADDR_TEST_INTERCEPT);
    intercept_disable();
}

void test_fan_callback()
{
    test_reset_para(ADDR_TEST_FAN);
    fan_work_disable();
}
void test_beep_callback()
{
    test_reset_para(ADDR_TEST_BEEP);
    alert_beep_disable();
}

void count_down_init_all_task()
{
    count_down_init(&fan_start_deceted, fan_start_deceted_callback);
    count_down_init(&feed_suction, feed_suction_callback);
    count_down_init(&feed_discharge, feed_discharge_callback);
    count_down_init(&feed_intercept, feed_intercept_callback);
    count_down_init(&spray_wash, spray_wash_callback);
    count_down_init(&spray_wash_between, spray_wash_between_callback);
    count_down_init(&spray_wash_stop, spray_wash_stop_callback);

// test init
    count_down_init(&test_spray_wash, test_spray_wash_callback);
    count_down_init(&test_suction, test_suction_callback);
    count_down_init(&test_intercept, test_intercept_callback);
    count_down_init(&test_fan, test_fan_callback);
    count_down_init(&test_beep, test_beep_callback);

// system
    count_down_init(&system_back_home, every_minute_no_operate_call_back);
    count_down_init(&password_no_identified, every_minute_no_identify_call_back);
}


void valve_disable()
{
    spray_wash_disable();
    discharge_enable();
    intercept_disable();
    fan_work_disable();
}

void valve_control_io_init()
{
    push_pull(0, 0x0F);
    valve_disable();
}


void tm0_isr() interrupt TIMER0_VECTOR  using 1
{   
    INT_PROC |= TICK_DUTY;
    //
    if (!--second_task_interval) {
        INT_PROC |= SECOND_DUTY;
        second_task_interval = COUNTER_PER_SECOND;
    }

    // todo 
    if(lack_pre ^ ALERT_FEED_LACK)
    {
        // rise
        if(ALERT_FEED_LACK)
        {
            time_not_exist = 0;
            time_feed_full = 0;
            signal_rise_fall = 1;
        }
        // fall
        else
        {
             signal_rise_fall = 0;
             // 料满停机时间置零
             time_feed_full = 0;
             // 1 料满停机结束
            if(feed_status == full)
            {
                signal_feed_full_delay = YES;
                time_task_interval = 0;
                alert_for_system &= ~ALTER_FULL_FEED;
            }

            // 2 放料过程中, 直到放料时间结束（检测缺料时间）
            // feed_lack_signal_set 是否进行检测
            if(feed_lack_signal_set)
            {
                if(feed_status == discharge)
                    signal_detect_enable = NO;
            }
            else
                signal_detect_enable = NO;

             // 3 放料已经结束,等待结束信号
             if(feed_status == wait_for_discharge_end)
             {
                signal_feed_lack_keep = YES;
                time_feed_lack_keep = 0; 
             }
        }
    }

    if(signal_rise_fall)
    {
        time_feed_full ++;
        if(time_feed_full == time_feed_stop * COUNTER_PER_SECOND)
        {
            if(feed_status == suction)
                INT_PROC |= FEED_SUCTION_END_DUTY;
        }
        if(time_feed_full == time_fan_delay_stop_set * COUNTER_PER_SECOND)
        {
            // 停止探测
            signal_detect_enable = NO;
            time_not_exist = 0;
            count_down_cancle_all_task();
            INT_PROC |= ALTER_FOR_FEED_FULL_DUTY;
        }
        // 缺料检测
        if(signal_detect_enable)
        {
            time_not_exist ++;
        }
	}
    else
    {
        // 缺料延迟启动
        if(signal_feed_full_delay)
        {
            time_task_interval ++;
            if(time_task_interval >= time_fan_delay_full_set * COUNTER_PER_SECOND)
            {
                alert_for_system &= ~ALTER_FULL_FEED;
                time_task_interval = 0;
                signal_feed_full_delay = NO;
                start_fan_signal = 1;
                system_start_or_not();
            }                 
        }

        // 确认没有料
        if(signal_feed_lack_keep)
        {
            time_feed_lack_keep ++;
            if(time_feed_lack_keep >= 25)
            {
                INT_PROC |= SPRAY_WASH_DUTY;
                signal_feed_lack_keep = NO;
                time_feed_lack_keep = 0; 
            }
        }
    }
    lack_pre = ALERT_FEED_LACK;
}