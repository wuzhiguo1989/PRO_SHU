#include "key.h"
#include "utils.h"
#include "string.h"
#include "share.h"
#include "time.h"
#include "sync.h"

static volatile u8 g_PreKey = 0;   //前次按键值 
static volatile u8 g_NowKey = 0;   //当前按键值 
volatile u8 g_ShortKeyCode = 0;    //短按键,单次触发 
volatile u8 g_LongKeyCode = 0;     //长按键,持续触发
volatile u8 g_KeyCode = 0;     //长按键,持续触发


u16 pca_next_target = 0;


void speak_init_params() {
    CCON = 0;                       //初始化PCA控制寄存器
                                    //PCA定时器停止
                                    //清除CF标志
                                    //清除模块中断标志
    CL = 0;
    CH = 0;
    pca_next_target =  pca_next_target + T4kHz;
    CCAP0L = pca_next_target;
    CCAP0H = pca_next_target >> 8;
    CMOD = 0x00;                    //设置PCA时钟源
                                    //禁止PCA定时器溢出中断
    CCAPM0 = 0x49;                  //PCA模块0为16位定时器模式
    CR = 1;
}



void key_io_init()
{
    // P32
    pure_input(3, 0x40);
    // P14
    pure_input(1, 0x10);
    // P15
    pure_input(1, 0x20);
    // P54
    push_pull(5, 0x10);
    // P17
    push_pull(1, 0x80);
    // P16
    push_pull(1, 0x40);

	POUT = 0;

    MUTE = 0;

    SPEAK = 0;


}


// 行列扫描的缺陷：当有多个键同时按下时，可能无法检测具体位置
// 不使用行列扫描，单独置行， 求列， 能否求出每个按键的位置？
u8 IO_KeyScan_G() {
    u8 key = 0xFF;
    key = key << 1 | MB;
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



//判断按键位置
//0x01 MB_ID按下

// status_mb_key 0 没有按下的状态
//               1 5s内等待倒计时
//               2 倒计时开始
//               3 倒计时暂停
//               4 倒计时继续
void check_key_location() {
    if(g_ShortKeyCode == MB_ID)
    {
        if(!STY || !CHG)
        {
            status_mb_key = 0;
            POUT = 0;
        }
		else
		{
            status_mb_key ++;
            speak_mode_set &= ~0x03;
            if(status_mb_key == 1)
            {
            // 开启5s内定时
                enter_5s_countdown = YES;
                reset_countdown_5s();
                time_init();
                time_colon_icon = 1;
                countdown_signal_start = NO;
                mute_beep_enable();
                reset_countdown_speak();
                speak_mode_set |= 0x01;
            }
            else if(status_mb_key == 2)
            {
            // 美白倒计时
                enter_5s_countdown = NO;
                // time_init();
                countdown_signal_start = YES;
                POUT = 1;
                mute_beep_enable();
                reset_countdown_speak();
                speak_mode_set |= 0x01;    
            }
            else if(status_mb_key == 3)
            {
            // 定时时间暂停
                countdown_signal_start = NO;
                time_colon_icon = 1;
                POUT = 0;
                mute_beep_enable();
                reset_countdown_speak();
                speak_mode_set |= 0x01;
            }
            else if(status_mb_key == 4)
            {
            // 定时时间继续
                countdown_signal_start = YES;
            // 恢复成2
                status_mb_key = 2;
                POUT = 1;
                mute_beep_enable();
                reset_countdown_speak();
                speak_mode_set |= 0x01;    
            }
		}
					
    }
    if(g_KeyCode == 0 && g_LongKeyCode == MB_ID)
    {
		
        if(status_mb_key == 1)
        {
            speak_mode_set &= ~0x03;
            if(local_time.minute == 30)
                local_time.minute = 0;
            local_time.minute = local_time.minute + 5;
            reset_countdown_5s();
            mute_beep_enable();
            reset_countdown_speak();
            speak_mode_set |= 0x01;
        }
        if(status_mb_key == 3)
        {
            speak_mode_set &= ~0x03;
            local_time.second = 0;
            if(local_time.minute == 30)
                local_time.minute = 0;
            local_time.minute = local_time.minute + 5;
            mute_beep_enable();
            reset_countdown_speak();
            speak_mode_set |= 0x01;
        }
    }
    g_KeyCode = g_LongKeyCode;
}


void key_duty_task() {
    key_scan_task();
    check_key_location();
}


//PCA中断 speak发声
void pca_isr() interrupt 7 using 1 
{
    CCF0 = 0;       //  清中断标志
    if(!speak_mode_set){
		speak_beep_disable(); 
        mute_beep_disable();
        pca_next_target =  pca_next_target + T4kHz;
        CCAP0L = pca_next_target;
        CCAP0H = pca_next_target >> 8;       
    } else {
        speak_beep_enable();
        pca_next_target =  pca_next_target + T1000Hz;
        CCAP0L = pca_next_target;
        CCAP0H = pca_next_target >> 8;
   }
}
