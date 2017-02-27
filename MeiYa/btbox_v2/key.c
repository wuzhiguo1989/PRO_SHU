#include "key.h"
#include "utils.h"
#include "string.h"
#include "share.h"
#include "time.h"
#include "sync.h"
#include "adc.h"

static volatile u8 g_PreKey = 0;   //前次按键值 
static volatile u8 g_NowKey = 0;   //当前按键值 
volatile u8 g_ShortKeyCode = 0;    //短按键,单次触发 
volatile u8 g_LongKeyCode = 0;     //长按键,持续触发
volatile u8 g_KeyCode = 0;     //长按键,持续触发



void system_io_init()
{
    // P14（DOWN）
    pure_input(1, 0x10);
    // P13 (UP)
    pure_input(1, 0x08); 
    // P16 (LOCK)
    pure_input(1, 0x40);        
    // P15 (CHG) 
    pure_input(1, 0x20);
    // P32（INT0）
    pure_input(3, 0x04);
    // P33 (INT1)
    pure_input(3, 0x08);
    // P17 (电池电压测量口)
    pure_input(1, 0x80);
    // P23 (指示灯驱动)
    push_pull(2, 0x08);
    // P54 (POUT)
    push_pull(5, 0x10);
    // P30 (800K)
    push_pull(3, 0x01);
    // P23 (+\-)
    push_pull(2, 0x08);
    // P55 (启动提示)
    push_pull(5, 0x20);
    // P31 程序启动
    push_pull(3, 0x02);
    // P12 美牙结束
    push_pull(1, 0x04);

    P55 = P31 = P12 = 1;

	POUT = 0;

    P23 = 0;
}




#define KEY_DOWN_ID 0x02
#define KEY_UP_ID   0x04
#define KEY_LOCK_ID 0x01


//g_NowKey ^ g_PreKey   :边缘触发 
//g_NowKey & (g_NowKey ^ g_PreKey):上升沿触发 
//g_PreKey & (g_NowKey ^ g_PreKey):下降沿触发 
//g_NowKey ^ (g_NowKey ^ g_PreKey):高电平触发(带消抖) 
//触发方式 ^ g_KeyCode :仅更新变化部分按键值 

//1.支持长短按键同时并行触发
//2.将短按键触发判据改为下降沿触发(短按键释放)



void key_scan_task() {
    u8 key = ~ADC_KeyScan_G();
    // u8 key = 0;
    static u16 CntPlus = 0;
    g_PreKey = g_NowKey; 
    g_NowKey = key;
    g_ShortKeyCode = (g_PreKey & (g_NowKey ^ g_PreKey)) ^ g_ShortKeyCode;  // 下降沿触发

    //if ((g_PreKey & (g_NowKey ^ g_PreKey)) ^ g_LongKeyCode)
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

void check_key_location() {
        if(g_ShortKeyCode == KEY_LOCK_ID)
        {
			// 倒计时开始
            status_mb_key ++;
            if(status_mb_key == 1)
            {
                reset_cnt_exit();
                adc_sampling =  NO;
            }
            if(status_mb_key == 2)
            {
                // 解锁
                status_mb_key = 0;
                adc_sampling = YES;		
            }
        }

        if(status_mb_key != 1)
        {
            if(g_ShortKeyCode == KEY_UP_ID)
            {
                //+
                local_time.minute ++;
                if(local_time.minute == 60)
                    local_time.minute = 0;
            }
            if(g_ShortKeyCode == KEY_DOWN_ID)
            {
                if(local_time.minute == 0)
                    local_time.minute = 60;
                local_time.minute --;
            }
            if(g_KeyCode == 0 && g_LongKeyCode == KEY_UP_ID)
            {
                //快速+开始
                increase_fast_refresh = YES;
            }
            if(g_KeyCode == KEY_UP_ID && g_LongKeyCode == 0)
            {   
                //快速+结束
                increase_fast_refresh = NO;
            }
            if(g_KeyCode == 0 && g_LongKeyCode == KEY_DOWN_ID)
            {
                //快速-开始
                decrease_fast_refresh = YES;
            }
            if(g_LongKeyCode == 0 && g_KeyCode == KEY_DOWN_ID)
            {   
                //快速-结束
                decrease_fast_refresh = NO;
            }
        }
        

        g_KeyCode = g_LongKeyCode;

}


void key_duty_task() {
        key_scan_task();
        check_key_location();
}