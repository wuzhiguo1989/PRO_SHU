#include "key.h"
#include "led.h"
#include "power.h"
#include "utils.h"
#include "ble.h"
#include "string.h"
#include "display.h"


#define LOW     0
#define HIGH    1

static volatile u8 g_PreKey = 0;   //前次按键值 
static volatile u8 g_NowKey = 0;   //当前按键值 
static volatile u8 g_ShortKeyCode = 0;    //短按键,单次触发 
static volatile u8 g_LongKeyCode = 0;     //长按键,持续触发 


void key_io_init()
{
    // P02~P03
    quasi_bidirectional(0, 0x04);
	quasi_bidirectional(0, 0x08);
}


// 行列扫描的缺陷：当有多个键同时按下时，可能无法检测具体位置
// 不使用行列扫描，单独置行， 求列， 能否求出每个按键的位置？
u8 IO_KeyScan_G() {
    u8 key = 0;
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
    u8 key = ~IO_KeyScan_G();
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
        // if(CntPlus > SHORT_KEY_TIME) 
        //     g_ShortKeyCode &= g_NowKey;

    } 
    else 
    {       
        CntPlus = 0;                                        //无新增按键出现,计数归零 
        g_LongKeyCode &= g_NowKey;                          //剔除已释放的按键
        g_ShortKeyCode &= g_NowKey;
    }

}

//判断按键位置
//0x01 LIGHT 按下
//0x02 POWER 按下
void check_key_location() { 
    if(g_ShortKeyCode == LIGHT_ID)
    {
        led_mode_set ++;
        if(led_mode_set == 3 || led_mode_set == 4)
            led_mode_set = 0;
        ctrl_mode_changed(led_mode_set);
    }
    else if(g_ShortKeyCode == POWER_ID)
    {
        power_status = ~power_status;
        if(!power_status)
            POWER_IN = 0;
        else
            POWER_IN = 1;
    }

}


void key_duty_task() {
    key_scan_task();
    check_key_location();
}


