#include "key.h"
#include "power.h"
#include "utils.h"
#include "string.h"
#include "lm567.h"
#include "stc15wxxxx.h"
#include "beep.h"
#include "task.h"

#define COUNTER_PER_SECOND 50

// volatile u8 resend_lock_command_signal = 0;
volatile u8 open_lock_ack_signal = 1;
volatile u8 lock_on_detect_signal = 1; 
volatile u8 lock_off_detect_signal = 1;
volatile u16 alter_time = 150;
volatile u8 alter_flag = 1;
volatile cycle_control_t xdata led_flicker;
volatile disp_t xdata led_mode_set;

static u8 xdata cnt_for_beep = 0; 

void led_display_disable() 
{
    led_power_disable();  
}

void led_io_init()
{
    // P54
    push_pull(5, 0x10);
    // P55
    push_pull(5, 0x20);
    // P14
    push_pull(1, 0x10);

    led_display_disable();
}


count_down_t xdata lock_count_down;
count_down_t xdata open_count_down;
count_down_t xdata alter_beep_count_down;
count_down_t xdata trace_beep_count_down;
// count_down_t xdata schedule_beep_count_down;
count_down_t xdata trace_mute_count_down;
// count_down_t xdata schedule_mute_count_down;
count_down_t xdata sleep_count_down;
count_down_t xdata open_beep_count_down;
count_down_t xdata lock_beep_count_down;
count_down_t xdata schedule_count_down;
count_down_t xdata uart_wakeup_count_down;
count_down_t xdata open_ack_count_down;
count_down_t xdata open_ack_lock_count_down;



void uart_wakeup_count_down_callback(){
    cpu_wake_up_disable();
    if(uart_id == open_success){
        uart_send_open_success();
    } else if(uart_id == open_fail){
        uart_send_open_fail();
    } else if(uart_id == lock_success){
        uart_send_lock_by_hand_success();
    } else if(uart_id == lock_fail){
        uart_send_lock_by_hand_fail();
    } else if(uart_id == beep_stop){
        uart_send_beep_stop();
    } else if(uart_id == status_inqury){
        uart_send_status_now();
    }
    reload_sleep_count_down();
}

void lock_count_down_callback() 
{
    // 0：关  1：开
    power_work_disable();
    if (!lock_off_detect_signal && !status_lock_detect()){
        EX1 = ENABLE;
        uart_id = lock_success;
        reload_uart_wakeup_count_down();
        status = off;
    } else {
        EX1 = ENABLE;
        uart_id = lock_fail;
        reload_uart_wakeup_count_down();
        status = on;
    }
    lock_off_detect_signal = 1;
    k_power_disable();
}


void open_count_down_callback() 
{ 
     // 0：关  1：开
    power_work_disable();
    if(!lock_on_detect_signal && INT1){
        // 开锁成功上报
        status = on;
        uart_id = open_success;
        k_power_disable();
        reload_uart_wakeup_count_down();
    } else {
        // status = off;
        // uart_id = open_fail;
        // reload_uart_wakeup_count_down();
        // todo:等待两秒去关锁
        // 两秒倒计时
        reload_open_ack_count_down(); 
    }
    lock_on_detect_signal = 1;   
}

void open_ack_count_down_callback(){
    // 开启检测开关
    cnt_off_time_plus = 0;
    lock_off_detect_signal = 0;
    k_power_enable();
    // todo: 推电机倒计时
    reload_open_ack_lock_count_down();

}

void open_ack_lock_count_down_calllback(){
    // 0：关  1：开
    if(!status_lock_detect()){
        //
        status = off;
        uart_id = open_fail;
        reload_uart_wakeup_count_down();  
    } else {
        //
        status = on;
        uart_id = open_success;
        reload_uart_wakeup_count_down(); 
    }
    k_power_disable();
    power_work_disable();
    lock_off_detect_signal = 1;
}


void open_beep_count_down_callback(){
    speak_mode = mute; 
    speak_beep_disable();
}

void lock_beep_count_down_callback(){
    speak_mode = mute; 
    speak_beep_disable();
}

void alter_beep_count_down_callback()
{
    speak_beep_disable();
    speak_mode = mute;
    if(!alter_flag)
    {
        uart_id = beep_stop;
        reload_uart_wakeup_count_down();
    } else{
        reload_sleep_count_down();
    }
}

void trace_beep_count_down_callback()
{
    cnt_for_beep ++;
    if(cnt_for_beep >= 3){ 
       speak_beep_disable();
       speak_mode = mute;
       // reload_sleep_count_down(); 
    } else{
       reload_trace_mute_count_down(); 
    }
}


void trace_mute_count_down_callback()
{
    reload_trace_beep_count_down();
}


void sleep_count_down_call_back()
{
    INT_PROC |= SLEEP_DUTY;
}

// void resend_lock_command_call_back()
// {
//     if(!resend_lock_command_signal){
//         // 收到、准备睡眠
//         reload_sleep_count_down(); 
//     } else if(resend_lock_command_signal == 1){
//         // 重新发送成功
//         uart_send_lock_by_hand_success();
//         reload_resend_lock_command();
//     } else {
//         // 重新发送失败
//         uart_send_lock_by_hand_fail();
//         reload_resend_lock_command();
//     } 
// }




void count_down_init(count_down_t xdata *cnt, event_cb func) {
    cnt -> second = 0;
    cnt -> millisecond = 0;
    cnt -> p_callback = func;
}

void count_down_reload(count_down_t xdata *cnt, const u16 second, const u8 millisecond) {
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


void count_down_at_all_task()
{
    count_down_at(&lock_count_down);
    count_down_at(&open_count_down);
    count_down_at(&alter_beep_count_down);
    count_down_at(&trace_beep_count_down);
    // count_down_at(&schedule_beep_count_down);
    count_down_at(&trace_mute_count_down);
    // count_down_at(&schedule_mute_count_down);
    count_down_at(&sleep_count_down);
    count_down_at(&open_beep_count_down);
    count_down_at(&lock_beep_count_down);
    count_down_at(&uart_wakeup_count_down);
    count_down_at(&open_ack_count_down);
    count_down_at(&open_ack_lock_count_down);
    // count_down_at(&resend_command_count_down);
}

void count_down_init_all_task()
{
    count_down_init(&lock_count_down, lock_count_down_callback);
    count_down_init(&open_count_down, open_count_down_callback);
    count_down_init(&alter_beep_count_down,alter_beep_count_down_callback);
    count_down_init(&trace_beep_count_down, trace_beep_count_down_callback);
    // count_down_init(&schedule_beep_count_down, schedule_beep_count_down_callback);
    count_down_init(&trace_mute_count_down, trace_mute_count_down_callback);
    // count_down_init(&schedule_mute_count_down, schedule_mute_count_down_callback);
    count_down_init(&sleep_count_down,
        sleep_count_down_call_back);
    count_down_init(&open_beep_count_down,
        open_beep_count_down_callback);
    count_down_init(&lock_beep_count_down,
        lock_beep_count_down_callback);
    count_down_init(&uart_wakeup_count_down, uart_wakeup_count_down_callback);
    count_down_init(&open_ack_count_down, open_ack_count_down_callback);
    count_down_init(&open_ack_lock_count_down,open_ack_lock_count_down_calllback);
    // count_down_init(&resend_command_count_down,resend_lock_command_call_back);

}

void reload_open_ack_count_down(){
   count_down_reload(&open_ack_count_down, 2, 0);  
}

void reload_open_ack_lock_count_down(){
    power_lock_work();
    open_lock_ack_signal = 1;
    count_down_reload(&open_ack_lock_count_down, 3, 0); 
}

void reload_lock_count_down() 
{
    power_lock_work();   
    open_lock_ack_signal = 0; 
    count_down_reload(&lock_count_down, 3, 0);

}

void reload_open_count_down() 
{ 
   count_down_reload(&open_count_down, 3, 0);
   power_open_work();
}

void reload_open_beep_count_down()
{
    speak_mode = open_beep; 
    count_down_reload(&open_beep_count_down, 0, 10); 
}

void reload_lock_beep_count_down()
{
    speak_mode = lock_beep; 
    count_down_reload(&lock_beep_count_down, 0, 10); 
} 

void reload_alter_beep_count_down()
{
    // 3
    count_down_reload(&alter_beep_count_down, alter_time / 50, alter_time % 50); 
}

void reload_sleep_count_down()
{
    // 1
    count_down_reload(&sleep_count_down, 3, 0); 
}

void reload_lock_sleep_count_down()
{
    // 5
    count_down_reload(&sleep_count_down, 8, 0); 
}

void reload_open_sleep_count_down()
{
    // 3
    count_down_reload(&sleep_count_down, 5, 0); 
}

void reload_alter_sleep_count_down()
{
    count_down_reload(&sleep_count_down, 6, 0); 
}

void reload_schedule_sleep_count_down()
{
    count_down_reload(&sleep_count_down, 900, 0);
}

void reload_uart_wakeup_count_down()
{
    cpu_wake_up_enable();
    count_down_reload(&uart_wakeup_count_down, 0, 10);
}

void cancel_open_ack_count_down(){
    count_down_cancel(&open_ack_count_down);
}

void cancel_open_ack_lock_count_down(){
    count_down_cancel(&open_ack_lock_count_down);
}


void cancle_sleep_count_down()
{
    count_down_cancel(&sleep_count_down);
}

void cancle_lock_count_down()
{
    count_down_cancel(&lock_count_down);
}

void cancle_open_count_down()
{
    count_down_cancel(&open_count_down);
}

void cancle_alter_count_down()
{
    count_down_cancel(&alter_beep_count_down);
}


void reload_cnt_for_beep() 
{
   cnt_for_beep = 0;
}

void reload_trace_beep_count_down() 
{
  speak_mode = trace_beep;
  count_down_reload(&trace_beep_count_down, 0, 25); 
}

void reload_trace_mute_count_down()
{
   speak_beep_disable();
   speak_mode = mute;
   count_down_reload(&trace_mute_count_down, 0, 25); 
}

// void reload_resend_lock_command()
// {
//     count_down_reload(&resend_command_count_down, 1, 0);   
// }



void init_sync_object() {
    led_flicker.cycle_enable = OFF;
    led_flicker.counter_for_cycle = 0;
}


static void led_display_off_driver_init();        
static void schedule_driver_init();  
static void trace_driver_init();         
static void lock_driver_init();               
static void open_driver_init(); 

static void led_display_off_half_cycle_reached();  
static void schedule_half_cycle_reached(); 
static void trace_half_cycle_reached(); 
static void lock_half_cycle_reached(); 
static void open_half_cycle_reached();

// idata *display_driver -> detailed display_driver
static  code driver_init_pt const driver_init[5] =
{
    led_display_off_driver_init,    
    schedule_driver_init,                     
    trace_driver_init,                  
    lock_driver_init,                     
    open_driver_init                     
};



static code half_cycle_reached_pt const half_cycle_callback[5] = 
{
    NULL,     
    schedule_half_cycle_reached,         
    trace_half_cycle_reached,          
    lock_half_cycle_reached,
    open_half_cycle_reached  
};

static idata u8 const cycle_param[5] = {0, 200, 100, 50, 50};

u8 cnt_for_cycle = 0;

void cycle_based_control() {
    // cycle_control_t
    if (led_flicker.cycle_enable) {
        ++led_flicker.counter_for_cycle;
    }

}


static void half_cycle_reached() {
    if (half_cycle_callback[led_mode_set])
        half_cycle_callback[led_mode_set]();
    led_flicker.counter_for_cycle = 0;
}

static void led_cycle_based_adjust() {
    if (led_flicker.cycle_enable) {
        if (led_flicker.counter_for_cycle >= cycle_param[led_mode_set])
            half_cycle_reached();

    } else {
        led_flicker.counter_for_cycle = 0;
    }
}


void led_mode_changed() {
    driver_init_pt led_mode_driver_init = driver_init[led_mode_set];
    if (led_mode_driver_init)
        led_mode_driver_init();
    led_flicker.counter_for_cycle = 0;
} 
// ------------------------- public -------------------------
void display_startup_init() {
    led_display_off_driver_init();
}

void led_display_task()
{
    led_cycle_based_adjust();
}


static void led_display_off_driver_init(){
      led_display_disable();
}

u8 cnt_for_cycle_schedule = 0;
bit flag_reverse = 0;
static void schedule_driver_init() {
    led_flicker.cycle_enable = ENABLE;
    cnt_for_cycle_schedule = 0;
    led_red_power_enable();
}

static void schedule_half_cycle_reached() {
    ++ cnt_for_cycle_schedule;
    if(cnt_for_cycle_schedule == 1){
        led_power_disable();
    }else if(cnt_for_cycle_schedule == 4){
        led_red_power_enable();
        cnt_for_cycle_schedule = 0;
    }
}

u8 cnt_for_cycle_trace = 0;
static void trace_driver_init() {
    led_flicker.cycle_enable = ENABLE;
    cnt_for_cycle_trace = 0;
    led_blue_power_enable();
}



static void trace_half_cycle_reached() {
    ++ cnt_for_cycle_trace;
    if(cnt_for_cycle_trace == 1){
        led_power_disable();
    } else if(cnt_for_cycle_trace == 2){
        led_blue_power_enable();
    } else if(cnt_for_cycle_trace == 3){
        led_power_disable();
    } else if(cnt_for_cycle_trace == 4){
        led_blue_power_enable();
    } else if(cnt_for_cycle_trace == 5){
        led_flicker.cycle_enable = OFF;
        led_power_disable();
        cnt_for_cycle_trace = 0;
        led_mode_set = led_display_off;
    }

}


static void lock_driver_init() {
    led_flicker.cycle_enable = ENABLE;
    led_blue_power_enable();
}

static void lock_half_cycle_reached() {
    led_flicker.cycle_enable = OFF;
    led_power_disable();
    cnt_for_cycle = 0;
    led_mode_set = led_display_off;
}


static void open_driver_init() {
    led_flicker.cycle_enable = ENABLE;
    led_green_power_enable();
}

static void open_half_cycle_reached() {
        led_flicker.cycle_enable = OFF;
        led_power_disable();
        cnt_for_cycle = 0;
        led_mode_set = led_display_off;
        led_mode_changed();

}


void led_display_stop()
{
    led_flicker.cycle_enable = OFF;
    led_power_disable();
    cnt_for_cycle = 0;
    led_mode_set = led_display_off;
}
