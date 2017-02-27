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

volatile bool sigal_in_use = 0;

volatile u8 sigal_available = 0;
volatile u8 time_read_flag = 0;

volatile u8 time_year = 0;
volatile u8 time_month = 0;
volatile u8 time_day = 0;

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


void system_start_or_not()
{
    // 温度、过载没解除
    if(!sigal_available){
        sigal_in_use = 1; 
        if(alert_for_system & 0x14)
            start_fan_signal = 0;
    // if(alert_for_system == ALTER_FULL_FEED)
    // {
    //     signal_feed_full_delay = YES;
    //     // regard as rise 
    //     signal_rise_fall = 1;
    // }
        if(!start_fan_signal)
            INT_PROC |= SYSTEM_END_DUTY;
        else
            INT_PROC |= START_DUTY;
    } else {
        start_fan_signal = 0;
        INT_PROC |= SYSTEM_END_DUTY;
    }    
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
count_down_t xdata feed_suction;
count_down_t xdata feed_discharge;
count_down_t xdata feed_intercept;
count_down_t xdata spray_wash;
count_down_t xdata spray_wash_between;
count_down_t xdata spray_wash_stop;

count_down_t xdata test_spray_wash;
count_down_t xdata test_suction;
count_down_t xdata test_intercept;
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
	count_down_at(&fan_start_deceted);
    count_down_at(&feed_discharge);
    count_down_at(&feed_suction);
    count_down_at(&feed_intercept);
    count_down_at(&spray_wash);
    count_down_at(&spray_wash_between);
    count_down_at(&spray_wash_stop);
// test
    count_down_at(&test_spray_wash);
    count_down_at(&test_suction);
    count_down_at(&test_intercept);
    count_down_at(&test_fan);
    count_down_at(&test_beep);
// system
    count_down_at(&system_back_home);
    count_down_at(&password_no_identified);
}

void count_down_cancle_all_task(){
    count_down_cancel(&fan_start_deceted);
    count_down_cancel(&feed_suction);
    count_down_cancel(&feed_discharge);
    count_down_cancel(&feed_intercept);
    count_down_cancel(&spray_wash);
    count_down_cancel(&spray_wash_between);
    count_down_cancel(&spray_wash_stop);
    start_fan_signal = 0;
}


void count_down_cancel_test()
{
    count_down_cancel(&test_spray_wash);
    count_down_cancel(&test_suction);
    count_down_cancel(&test_intercept);
    count_down_cancel(&test_fan);
    count_down_cancel(&test_beep);
}


void spray_wash_start_callback()
{
    printf("spray_wash, PoW\r\n");
    spray_wash_enable();
    reload_spray_wash();
}

void detected_feed_full_start()
{
    if(!ALERT_FEED_LACK)
    {
        printf("PoW1\r\n");
        signal_feed_full_delay = NO;
        reload_fan_start_deceted();
        fan_work_enable();
        system_enter_fan_work();
        alert_for_system &= ~ALTER_FULL_FEED;
    }
    else
    {
        signal_feed_full_delay = YES;
        // regard as rise 
        signal_rise_fall = 1;
        system_enter_alter_feed_full();
        alert_for_system &= ~ALTER_LACK_FEED;
        alert_for_system |= ALTER_FULL_FEED;
    }
}

// 开始截料
void feed_intercept_callback()
{
    printf("FEED_INTERCEPT, PoW\r\n");
    intercept_enable();
    system_enter_intercept();
}

// 吸料
void feed_suction_task()
{
    // 吸料
    suction_enable();
    // 开始记录料满时间
    time_feed_full = 0;
    // 截料时间 >= 吸料时间
    if(!time_feed_intercept_set)
        feed_intercept_callback();
    else 
    {
        printf("FEED_SUCTION, PoW\r\n");
        system_enter_sunction();
    }

    if(time_feed_intercept_set != time_feed_suction_set)
        reload_feed_intercept();
    reload_feed_suction();
}


// 启动时间到
void fan_start_deceted_callback()
{
    if(update_data_should)
    {
        // 所有的值改变一下
        updata_data_from_dwscreen();
        update_data_should = false;
    }
    counter_for_spray_wash_logic = counter_for_spray_wash_set;
    if(counter_for_spray_wash_set)
        // reload_spray_wash_start();
        spray_wash_start_callback();
    else
        feed_suction_task();
}





// 吸料时间到、进放料
void feed_suction_callback()
{
    printf("FEED_DISCHARGE\r\n");
    intercept_disable();
    //放料
    discharge_enable();
    //放料动画
    system_enter_discharge();
    reload_feed_discharge();
    //检测是否料满过
    // 没有料满
    if(feed_lack_signal_set)
    {
        signal_detect_enable = YES;
        time_not_exist = 0;
    }
    else
    {
        signal_detect_enable = NO;
        time_not_exist = 0;
    }
}




void spray_wash_callback()
{
    //关
    printf("dengdai\r\n");
    spray_wash_disable();
    counter_for_spray_wash_logic --;
    if(!counter_for_spray_wash_logic)
        reload_spray_wash_stop();
    else
        reload_spray_wash_between();
}

void spray_wash_between_callback()
{
    spray_wash_enable();
    reload_spray_wash();
}

void spray_wash_stop_callback()
{
    // 等待确认
    printf("guangji\r\n");
    feed_suction_task();
}

void start_spray_wash()
{
    if(update_data_should)
    {
        // 所有的值改变一下
        updata_data_from_dwscreen();
        update_data_should = false;
    }
    counter_for_spray_wash_logic = counter_for_spray_wash_set;
    if(counter_for_spray_wash_set)
        // reload_spray_wash_start();
        spray_wash_start_callback();
    else
        feed_suction_task();
}


// 放料时间到
void feed_discharge_callback()
{
    printf("g\r\n");
    if(feed_lack_signal_set)
    {
        if(time_not_exist < time_feed_lack_set * COUNTER_PER_100MS)
        // 缺料计数+1
        {
            counter_for_feed_lack ++;
            if(counter_for_feed_lack >= counter_for_lack_alter_set)
                alert_for_system |= ALTER_LACK_FEED;
            else
                alert_for_system &= ~ALTER_LACK_FEED;
            send_counter_for_feed_lack_to_dwscreen(counter_for_feed_lack);
        }
        else
        {
            alert_for_system &= ~ALTER_LACK_FEED;
            if(counter_for_feed_lack)
            {
                counter_for_feed_lack = 0;
                send_counter_for_feed_lack_to_dwscreen(counter_for_feed_lack);
            }
        }
    }
    else
    {
        if(!counter_for_feed_lack)
        {
            counter_for_feed_lack = 0;
            send_counter_for_feed_lack_to_dwscreen(counter_for_feed_lack);
        }
        alert_for_system &= ~ALTER_LACK_FEED;
    }

    time_not_exist = 0;
    signal_detect_enable = NO;
    // 说明没有关闭
    // 依旧料满
    if(ALERT_FEED_LACK)
    {
        feed_status = wait_for_discharge_end;
    }
    // 无料
    else
    // 反吹
        start_spray_wash();
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
    if(lack_pre ^ ALERT_FEED_LACK)
    {
        // rise
        if(ALERT_FEED_LACK && sigal_in_use)
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
        if(time_feed_full == time_feed_stop_set * COUNTER_PER_100MS)
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