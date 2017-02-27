#include "key.h"
#include "power.h"
#include "utils.h"
#include "string.h"
#include "lm567.h"
#include "stc15wxxxx.h"
#include "beep.h"
#include "task.h"

#define COUNTER_PER_SECOND 50

led_t led_config_logic[4];
volatile cycle_control_t xdata led_flicker;
volatile disp_t xdata led_mode_set;

static u8 xdata cnt_for_beep = 0; 

void led_display_disable() 
{
    led_power_disable();  
    LED_DATA = 0;
}

void led_io_init()
{
    // P14
    push_pull(1, 0x10);
    // P54
    push_pull(5, 0x10);
    // P55
    push_pull(5, 0x20);

    led_display_disable();
}



void T1_code()
{
    LED_DATA = 1;
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    LED_DATA = 0;
    _nop_();
    _nop_();
}

void T0_code()
{
    LED_DATA = 1;
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    LED_DATA = 0;
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
}


// // G3G2G1G0R3R2R1R0B3B2B1B0
void led_color_8bit(led_t *led_config)
{
    u8 flag, iterator;
    u8 mask = 0x80, t_code = led_config -> green; 
    for(iterator = 0; iterator < 8; ++iterator)
    {
        flag = (t_code & mask) >> 7;
        if(flag == 1){
            T1_code();
        } else {
            T0_code();
        }
        t_code <<= 1;
    }

    t_code = led_config -> red;
    for(iterator = 0; iterator < 8; ++iterator)
    {
        flag = (t_code & mask) >> 7;
        if(flag == 1){
            T1_code();
        } else {
            T0_code();
        }
        t_code <<= 1;
    }

    t_code = led_config -> blue;
    for(iterator = 0; iterator < 8; ++iterator)
    {
        flag = (t_code & mask) >> 7;
        if(flag == 1){
            T1_code();
        } else {
            T0_code();
        }
        t_code <<= 1;
    }
}


void led_config_change()
{
    u8 led_index;
    for(led_index = 0; led_index < LED_CNT; led_index ++)
        led_color_8bit(&led_config_logic[led_index]); 
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
count_down_t xdata schedule_count_down;

void lock_count_down_callback() 
{
	power_work_disable();
    EX1 = ENABLE;
    if(INT1)
    {
        INT_PROC |= LOCK_FAIL_DUTY;
    }
    else
    {
        // 关锁成功上报
        cpu_wake_up_enable();
        uart_send_lock_by_hand_success();
        cpu_wake_up_disable(); 
        status = off;
    }
}
void open_count_down_callback() 
{ 
	power_work_disable();
    // 开锁成功上报
    cpu_wake_up_enable();
    uart_send_open_success();
    cpu_wake_up_disable();
    status = on;
    EX1 = ENABLE;
    reload_sleep_count_down();
}


void open_beep_count_down_callback(){
    speak_mode = mute; 
    speak_beep_disable();
}

void alter_beep_count_down_callback()
{
    speak_beep_disable();
    speak_mode = mute;
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
}



void reload_lock_count_down() 
{
   count_down_reload(&lock_count_down, 2, 0); 
   power_lock_work();
}

void reload_open_count_down() 
{ 
   count_down_reload(&open_count_down, 1, 0);
   power_open_work();
}

void reload_open_beep_count_down()
{
    speak_beep_max();
    speak_mode = open_beep; 
    count_down_reload(&open_beep_count_down, 0, 10); 

}

void reload_alter_beep_count_down()
{
    // 5
    count_down_reload(&alter_beep_count_down, 5, 0); 
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
    count_down_reload(&sleep_count_down, 3, 0); 
}

void reload_alter_sleep_count_down()
{
    count_down_reload(&sleep_count_down, 6, 0); 
}

void reload_schedule_sleep_count_down()
{
    count_down_reload(&sleep_count_down, 900, 0);
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

void reload_cnt_for_beep() 
{
   cnt_for_beep = 0;
}

void reload_trace_beep_count_down() 
{
  speak_beep_mid();
  speak_mode = trace_beep;
  count_down_reload(&trace_beep_count_down, 0, 25); 
}

void reload_trace_mute_count_down()
{
   speak_beep_disable();
   speak_mode = mute;
   count_down_reload(&trace_mute_count_down, 0, 25); 
}



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

static idata u8 const cycle_param[5] = {0, 8, 4, 50, 50};

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
    led_config_change();
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
    led_config_change();
} 
// ------------------------- public -------------------------
void display_startup_init() {
    led_display_off_driver_init();
    led_config_change();
}

void led_display_task()
{
    led_cycle_based_adjust();
}


static void led_display_off_driver_init()
{
	  led_display_disable();
}

u8 cnt_for_cycle_schedule = 0;
bit flag_reverse = 0;
static void schedule_driver_init() {
    u8 i;
    led_flicker.cycle_enable = ENABLE;
    cnt_for_cycle_schedule = 0;
    flag_reverse = 0;
    led_power_enable();
    for(i = 0; i < LED_CNT; i++)
    {
        led_config_logic[i].green = 0;
        led_config_logic[i].red = 0;
        led_config_logic[i].blue = 0;
    }
}

static void schedule_half_cycle_reached() {
    u8 i;
    for(i = 0; i < LED_CNT; i++)
    {
        led_config_logic[i].red = 0;
        led_config_logic[i].green = 0;
        if(i == cnt_for_cycle_schedule)
        {
            if(!flag_reverse)
            {
                led_config_logic[i].blue = led_config_logic[i].blue + 5;
                if(led_config_logic[i].blue > 250)
                    flag_reverse = ~ flag_reverse;
            }
            else
            {
                led_config_logic[i].blue = led_config_logic[i].blue - 5;
                if(led_config_logic[i].blue == 0)
                {
                    flag_reverse = ~ flag_reverse;
                    cnt_for_cycle_schedule ++;
                    if(cnt_for_cycle_schedule == 4)
                        cnt_for_cycle_schedule = 0;
                }
            }
        }
        else
            led_config_logic[i].blue = 0;
    }
}

u8 cnt_for_cycle_trace = 0;
static void trace_driver_init() {
    u8 i;
    led_flicker.cycle_enable = ENABLE;
    cnt_for_cycle_trace = 0;
    led_power_enable();
    for(i = 0; i < LED_CNT; i++)
    {
        led_config_logic[i].green = 0;
        led_config_logic[i].red = 0;
        led_config_logic[i].blue = 0;
    }
}



static void trace_half_cycle_reached() {
    u8 i;
    for(i = 0; i < LED_CNT; i++)
    {
        led_config_logic[i].green = 0;
        led_config_logic[i].blue = 0;
        led_config_logic[i].red = led_config_logic[i].red + 5;
    }
    if(led_config_logic[0].red > 250)
    {
        led_config_logic[0].red = 0;
        led_config_logic[1].red = 0;
        led_config_logic[2].red = 0;
        cnt_for_cycle_trace ++;
    }
    if(cnt_for_cycle_trace == 3){
        led_flicker.cycle_enable = OFF;
        led_power_disable();
        cnt_for_cycle_trace = 0;
        led_mode_set = led_display_off;
            //led_mode_changed();
    }

}


static void lock_driver_init() {
	u8 i;
    led_flicker.cycle_enable = ENABLE;
    led_power_enable();
    cnt_for_cycle = 0;
    for(i = 0; i < LED_CNT; i++)
    {
        led_config_logic[i].red = 0;
        led_config_logic[i].blue = 0;
        led_config_logic[i].green = 255;
    }
}

static void lock_half_cycle_reached() {
    u8 i;
    ++ cnt_for_cycle;
    for(i = 0; i < LED_CNT; i++)
    {
        led_config_logic[i].red = 0;
        led_config_logic[i].green = 0;
        led_config_logic[i].blue = 0;
    }
    // 转圈第一个红
    if(cnt_for_cycle == 1){
        for(i = 0; i < LED_CNT; i++)
        {
        // 底色
            led_config_logic[i].green = 255;
        }
    } else if(cnt_for_cycle == 2){
        // led_config_logic[0].red = 255;
        led_config_logic[1].green = 255;
        led_config_logic[2].green = 255;
        led_config_logic[3].green = 255;
    } else if(cnt_for_cycle == 3){
        // led_config_logic[1].red = 255;
        led_config_logic[2].green = 255;
        led_config_logic[3].green = 255;
    } else if(cnt_for_cycle == 4){
        // led_config_logic[2].red = 255;
        led_config_logic[3].green = 255;
    } else if(cnt_for_cycle == 5){
        // led_config_logic[3].red = 255;
        led_flicker.cycle_enable = OFF;
        led_power_disable();
        cnt_for_cycle = 0;
        led_mode_set = led_display_off;
        led_mode_changed();
    } 
    // else if(cnt_for_cycle == 6){
    // // 时间到(待机、灯光闪烁结束)
    //     led_flicker.cycle_enable = OFF;
    //     led_power_disable();
    //     cnt_for_cycle = 0;
    //     led_mode_set = led_display_off;
    //     led_mode_changed();
    // }
}


static void open_driver_init() {
		u8 i;
    led_flicker.cycle_enable = ENABLE;
    led_power_enable();
    cnt_for_cycle = 0;
    for(i = 0; i < LED_CNT; i++)
    {
        led_config_logic[i].red = 0;
        led_config_logic[i].blue = 0;
        led_config_logic[i].green = 255;
    }
}

static void open_half_cycle_reached() {
    u8 i;
    ++ cnt_for_cycle;
    for(i = 0; i < LED_CNT; i++)
    {
        led_config_logic[i].red = 0;
        led_config_logic[i].green = 0;
        led_config_logic[i].blue = 0;
    }
    // 转圈第一个红
    if(cnt_for_cycle == 1){
        for(i = 0; i < LED_CNT; i++)
        {
            // led_config_logic[i].red = 255;
            led_config_logic[i].green = 255;
        }
    } else if(cnt_for_cycle == 2){
        // led_config_logic[0].blue = 255;
        led_config_logic[1].green = 255;
        led_config_logic[2].green = 255;
        led_config_logic[3].green = 255;
    } else if(cnt_for_cycle == 3){
        // led_config_logic[1].blue = 255;
        led_config_logic[2].green = 255;
        led_config_logic[3].green = 255;
    } else if(cnt_for_cycle == 4){
        // led_config_logic[2].blue = 255;
        led_config_logic[3].green = 255;
    } else if(cnt_for_cycle == 5){
        // led_config_logic[3].blue = 255;
        led_flicker.cycle_enable = OFF;
        led_power_disable();
        cnt_for_cycle = 0;
        led_mode_set = led_display_off;
        led_mode_changed();
    } 
    // else if(cnt_for_cycle == 6){
    // // 时间到(待机、灯光闪烁结束)
    //     led_flicker.cycle_enable = OFF;
    //     led_power_disable();
    //     cnt_for_cycle = 0;
    //     led_mode_set = led_display_off;
    //     led_mode_changed();
    //     // INT_PROC |= SLEEP_DUTY;
    //     // reload_sleep_count_down();
    // }
}


void led_display_stop()
{
    led_flicker.cycle_enable = OFF;
    led_power_disable();
    cnt_for_cycle = 0;
    led_mode_set = led_display_off;
    led_mode_changed();
}
