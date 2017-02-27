#include "interact.h"
#include "lm567.h"
#include "utils.h"
#include "task.h"
#include "power.h"
#include "key.h"
#include "beep.h"

static u8 xdata g_PreKey; 	//前次按键值 
static u8 xdata g_NowKey; 	//当前按键值 
u8 g_ShortKeyCode = 0;	//短按键,单次触发 
u8 g_LongKeyCode = 0; 	//长按键,持续触发
u8 signal_lock_shake = 0; 


#define KEY_INT1  0x01

// KeyScan for common use without any speedup
// For each pin is separate
static u8 IO_KeyScan_G() {
	u8 key = 0xFE;
	key |= INT1;
	return key;
}

// 
// g_NowKey ^ g_PreKey	: 边缘触发 
// g_NowKey & (g_NowKey ^ g_PreKey): 上升沿触发 
// g_PreKey & (g_NowKey ^ g_PreKey): 下降沿触发 
// g_NowKey ^ (g_NowKey ^ g_PreKey): 高电平触发(带消抖) 
// 触发方式 ^ g_KeyCode : 仅更新变化部分按键值 

//1.支持长短按键同时并行触发
//2.将短按键触发判据改为下降沿触发(短按键释放)
void key_scan_task() {
	static u8 CntPlus = 0;
	
	g_PreKey = g_NowKey; 
	g_NowKey = ~IO_KeyScan_G();
	g_ShortKeyCode = (g_PreKey & (g_NowKey ^ g_PreKey)) ^ g_ShortKeyCode;  // 下降沿触发 

	if (g_NowKey ^ g_PreKey ^ g_NowKey ^ g_LongKeyCode) 		// 高电平触发(带消抖) 
	{ 
		if(CntPlus++ > LONG_KEY_TIME)						g_LongKeyCode |= g_NowKey;				
			// 添加长按键值  
		if(CntPlus > SHORT_KEY_TIME) 
			g_ShortKeyCode &= g_NowKey;
		if(CntPlus > 0 && !signal_lock_shake){
			reload_sleep_count_down();
		}
		if(CntPlus > 0){
			power_lock_work();
			signal_lock_shake = 1;
		}
	} 
	else 
	{ 		
		CntPlus = 0;
		if(signal_lock_shake){
			EX1 = ENABLE;
			power_work_disable();
			signal_lock_shake = 0;
		}								// 无新增按键出现,计数归零 
		g_LongKeyCode &= g_NowKey; 							// 剔除已释放的按键
		g_ShortKeyCode &= g_NowKey;
	}
}



u8 g_LongKeyCode_pre = 0;
void key_task_duty()
{
	key_scan_task();
	if(!g_LongKeyCode_pre && g_LongKeyCode){
		if(g_LongKeyCode & KEY_INT1){
			INT_PROC |= LOCK_DUTY;
			signal_lock_shake = 0;
	   }
    }
    g_LongKeyCode_pre = g_LongKeyCode;
}


