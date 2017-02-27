#include "key.h"
#include "led.h"
#include "power.h"
#include "utils.h"
#include "string.h"
#include "display.h"
#include "lm567.h"


#define LOW     0
#define HIGH    1

#define COUNTER_PER_SECOND 50

static volatile u16 g_PreKey = 0;   //前次按键值 
static volatile u16 g_NowKey = 0;   //当前按键值 
static volatile u16 g_ShortKeyCode = 0;    //短按键,单次触发 
static volatile u16 g_LongKeyCode = 0;     //长按键,持续触发
static volatile u16 g_LongKeyCodePre = 0; 


void Delay1000ms()      //@24.000MHz
{
    unsigned char i, j, k;

    _nop_();
    _nop_();
    i = 1;
    j = 2;
    k = 238;
    do
    {
        do
        {
            while (--k);
        } while (--j);
    } while (--i);
}


void key_io_init()
{
    // P11~P12
    pure_input(1, 0x02);
    pure_input(1, 0x04);
    // P00
    pure_input(0, 0x01);
    // P25~27
    pure_input(2, 0xC0);
    // P33
    pure_input(3, 0x08);
    pure_input(3, 0x10);
    pure_input(3, 0x20);
    // P01~P03
    push_pull(0,0x0E);
    // P10
    pure_input(1, 0x01);
    // pure_input(0,0x02);

    // 默认蓝牙关闭
    // 置低
    P01 = 0;
    P02 = 0;
    P03 = 0;

}


// 行列扫描的缺陷：当有多个键同时按下时，可能无法检测具体位置
// 不使用行列扫描，单独置行， 求列， 能否求出每个按键的位置？
u16 IO_KeyScan_G() {
    u16 key = 0;
    key = key << 1 | K9;
    key = key << 1 | K8;
    key = key << 1 | K7;
    key = key << 1 | K6;
    key = key << 1 | K5;
    key = key << 1 | K4;
    key = key << 1 | K3;
    key = key << 1 | K2;
    key = key << 1 | K1;
    return key;
}





//g_NowKey ^ g_PreKey   :边缘触发 
//g_NowKey & (g_NowKey ^ g_PreKey):上升沿触发 
//g_PreKey & (g_NowKey ^ g_PreKey):下降沿触发 
//g_NowKey ^ (g_NowKey ^ g_PreKey):高电平触发(带消抖) 
//触发方式 ^ g_KeyCode :仅更新变化部分按键值 

//1.支持长短按键同时并行触发
//2.将短按键触发判据改为下降沿触发(短按键释放)



void key_scan_task() {
    u16 key = ~IO_KeyScan_G() & 0x01FF;
    static u16 CntPlus = 0;
    g_PreKey = g_NowKey; 
    g_NowKey = key;
    g_ShortKeyCode = (g_PreKey & (g_NowKey ^ g_PreKey)) ^ g_ShortKeyCode;  // 下降沿触发


    if (g_NowKey ^ g_PreKey ^ g_NowKey ^ g_LongKeyCode)         // 高电平触发(带消抖) 
    { 
        if(CntPlus++ > LONG_KEY_TIME)
        {                           
            g_LongKeyCode |= g_NowKey;                          //添加长按键值
        }    
        if(CntPlus > SHORT_KEY_TIME) 
            g_ShortKeyCode &= g_NowKey;
    } 
    else 
    {       
        CntPlus = 0;                                        //无新增按键出现,计数归零 
        g_LongKeyCode &= g_NowKey;                          //剔除已释放的按键
        g_ShortKeyCode &= g_NowKey;
    }
}





count_down_t xdata volume_increase;
count_down_t xdata volume_decrease;
count_down_t xdata bluetooth_on_off;
count_down_t xdata bluetooth_pair;
//count_down_t xdata volume_increase_quick;
//count_down_t xdata volume_decrease_quick;

void volume_increase_callback()
{
    P03 = 0;
}
void volume_decrease_callback()
{
    P02 = 0;
}
void bluetooth_on_off_callback()
{
    P01 = 0;
}
void bluetooth_pair_callback()   
{
    P01 = 0;
}



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


void count_down_at_all_task()
{
    count_down_at(&volume_increase);
    count_down_at(&volume_decrease);
    count_down_at(&bluetooth_on_off);
    count_down_at(&bluetooth_pair);
    // count_down_at(&volume_increase_quick);
    // count_down_at(&volume_decrease_quick);
}

void count_down_init_all_task()
{
    count_down_init(&volume_increase, volume_increase_callback);
    count_down_init(&volume_decrease, volume_decrease_callback);
    count_down_init(&bluetooth_on_off, bluetooth_on_off_callback);
    count_down_init(&bluetooth_pair, bluetooth_pair_callback);
    // count_down_init(&volume_increase_quick, volume_increase_quick_callback);
    // count_down_init(&volume_decrease_quick, volume_decrease_quick_callback);
}



#define reload_volume_increase() count_down_reload(&volume_increase, 0, 6); P03 = 1
#define reload_volume_decrease() count_down_reload(&volume_decrease, 0, 6); P02 = 1
#define reload_bluetooth_on_off() count_down_reload(&bluetooth_on_off, 0, 10); P01 = 0
#define reload_bluetooth_pair() count_down_reload(&bluetooth_pair, 0, 6); P01 = 1


void short_key_func_dispatcher() { 
    if(g_ShortKeyCode == KEY_BT_ID){
        // 蓝牙打开、关闭交替
        // reload_bluetooth_on_off();
        if(sys_status == ON){
           P01 = ~P01; 
        }
    }else if(g_ShortKeyCode == KEY_POWER_ID){
        //
        sys_status = ~sys_status;
        if(sys_status == ON)
        {
            led_mode_set = 1;
            ctrl_mode_changed(led_mode_set);
            power_status = ON;
            power_level_set = 1;
        } else {
            P01 = 0;
            power_status = OFF;
            power_select_level();
            led_mode_set = 0;
            ctrl_mode_changed(led_mode_set);
        }
    }else if(g_ShortKeyCode == KEY_LAMP_ID){
        // 红色、绿色、蓝色、混色、七彩、关闭
        if(sys_status == ON)
        {
            led_mode_set ++;
            if(led_mode_set == 11)
                led_mode_set = 0;
            ctrl_mode_changed(led_mode_set);
        }

    }else if(g_ShortKeyCode == KEY_FOG_ID){
        // 1、2、3、4、关闭切换
        if(power_status == ON)
        {
            if(power_level_set == 0)
               power_level_set = 5;
            power_level_set --;
            power_select_level();
        }
    }else if(g_ShortKeyCode == KEY_VA_ID){
        // 音量+1
        reload_volume_increase();
    }else if(g_ShortKeyCode == KEY_VM_ID){
        // 音量-1
        reload_volume_decrease();
    }else if(g_ShortKeyCode == KEY_LIGHT_ID){
        led_mode_set ++;
        if(led_mode_set == 11)
            led_mode_set = 0;
        ctrl_mode_changed(led_mode_set);
    }else if(g_ShortKeyCode == KEY_BLUETOOTH_ID){
        P01 = ~P01;
    }else if(g_ShortKeyCode == KEY_MIST_ID){
        power_status = ~power_status;
        power_select_level();
    }

}


#define INTERVAL_BLUTOOTH_PAIR_HOLD   20
#define INTERVAL_VOLUME_INCREASE_HOLD 10
#define INTERVAL_VOLUME_DECREASE_HOLD 10
volatile u8 interval_bluetooth_pair_hold = INTERVAL_BLUTOOTH_PAIR_HOLD;
volatile u8 interval_volume_increase_hold = INTERVAL_VOLUME_INCREASE_HOLD;
volatile u8 interval_volume_decrease_hold = INTERVAL_VOLUME_DECREASE_HOLD;

void decrease_key_hold_interval()
{
    if(interval_volume_increase_hold)
        -- interval_volume_increase_hold;
    if(interval_volume_decrease_hold)
        -- interval_volume_decrease_hold;
}

void long_key_func_dispatcher() { 
    if(g_LongKeyCode == KEY_BLUETOOTH_ID && g_LongKeyCodePre == 0){
		//配对
        reload_bluetooth_pair();
    }else if(g_LongKeyCode == KEY_VA_ID){
        // 快速+
        if(!interval_volume_increase_hold)
        {
            interval_volume_increase_hold = INTERVAL_VOLUME_INCREASE_HOLD;
            reload_volume_increase();
        }
    }else if(g_LongKeyCode == KEY_VM_ID){
        // 快速-
        if(!interval_volume_decrease_hold)
        {
            interval_volume_decrease_hold = INTERVAL_VOLUME_DECREASE_HOLD;
            reload_volume_decrease();
        }
    }
}


void key_duty_task() {
    key_scan_task();
    if(g_ShortKeyCode)
    {
        // TX1_write2buff(g_ShortKeyCode);
        short_key_func_dispatcher();
    }
    if(g_LongKeyCode)
    {
        // TX1_write2buff(g_LongKeyCode);
        long_key_func_dispatcher();
    }
    g_LongKeyCodePre = g_LongKeyCode;
}


