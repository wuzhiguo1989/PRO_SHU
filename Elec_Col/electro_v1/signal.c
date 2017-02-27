#include "signal.h"
#include "task.h"
#include "share.h"
#include "beep.h"
#include "sync.h"
#include "light.h"
#include "time.h"
#include "jl17467pqw.h"
#include "control.h"


static u8 remote_signal = 0;

void key_on_scan()
{
	if(!KEY_ON && state_current == KEY_NORMAL_STATE)
	{
		Store_mileage();
		display_clean();
		INT_CLKO &= 0xEF;
		enter_lock_state_2s = YES;
		state_current = LOCK_STATE;
		reset_2s_counter_for_key();
		first_alert = 0;
    	BACK = 0;
    	RELAY = 0;
    	speak_mode_set &= ~0x0fff; //关闭所有声音
	}
}

void init_control_signal()
{
    INT1 = 1;
    IT1 = 0;                    //设置INT1的中断类型 (1:仅下降沿 0:上升沿和下降沿)
    EX1 = 1;                    //使能INT1中断
}


static void init_speak_param()
{
	interval = 0;
	speak_time = speak_time_interval[speak_mode - 1];
	mute_beep_enable();
	light_follow_speak();
}

void  wait_for_alert() 
{
    INT_CLKO &= 0xEF;
    enter_lock_state_2s = YES;
    reset_2s_counter();
}


void state_enter_trans(u8 control_signal)
{
	u8 state_current_pre = state_current;
	if(state_current_pre == LOCK_STATE)
	{

		// 正常键：进入正常状态
		if(control_signal == NORMAL_SIGNAL)
		{
			speak_mode_set &= ~0x003f;
			time_init();
			reset_time_counter();
			reset_speed_counter();
			mileage_init();
			BACK = 0;
			// state_current = NORMAL_STATE;
			// 解锁的铃声
			speak_mode_set |= 0x0001;
			speak_mode = unlockmode;
			init_speak_param();
		}
		// 震动信号：震动报警
		if(control_signal == SHOCK_SIGNAL)
		{
			speak_mode_set &= ~0x003f;
			state_current = ALERT_STATE;
			// 第一次震动 预报警4s
			if(!first_alert)
			{
				enter_lock_state_15s = NO;
				reset_10s_counter();
				speak_mode_set |= 0x0008;
				speak_mode = firstalertmode;
 			}
			// 10s以后震动 报警15s
			else
			{
				enter_lock_state_15s = NO;
				reset_10s_counter();
				speak_mode_set |= 0x0010;
				speak_mode = secondalertmode;
			}
			init_speak_param();
		}
		// 寻车键：提示车
		if(control_signal == LOOK_FOR_SINAL)
		{
			speak_mode_set &= ~0x003f;
			state_current = LOCK_STATE;
			speak_mode_set |= 0x0002;
			speak_mode = lookformode;
			init_speak_param();
//			wait_for_alert();
		}
		// 锁定键：锁定
		if(control_signal == LOCK_SIGNAL)
		{
			speak_mode_set &= ~0x003f;
			state_current = LOCK_STATE;
			first_alert = 0;
    		BACK = 0;
			speak_mode_set |= 0x0004;
			speak_mode = firstalertmode;
			init_speak_param();
//			wait_for_alert();
		}
		// 解锁键：解锁
		if(control_signal == UNLOCK_SIGNAL)
		{
			speak_mode_set &= ~0x003f;
			state_current = UN_LOCK_STATE;
			speak_mode_set |= 0x0020;
			speak_mode = buzzer;
			init_speak_param();
		}
		if(control_signal == KEY_SIGNAL)
		{
			time_init();
			reset_time_counter();
			reset_speed_counter();
			mileage_init();
			BACK = 1;
			RELAY = 1;
			state_current = KEY_NORMAL_STATE;
		}
	} 
	else if(state_current_pre == UN_LOCK_STATE){
		// 正常键：进入正常状态
		if(control_signal == NORMAL_SIGNAL)
		{
			speak_mode_set &= ~0x003f;
			time_init();
			reset_time_counter();
			reset_speed_counter();
			mileage_init();
			BACK = 0;
			// state_current = NORMAL_STATE;
			// 解锁的铃声
			speak_mode_set |= 0x0001;
			speak_mode = unlockmode;
			init_speak_param();
		}
		// 震动信号
		if(control_signal == SHOCK_SIGNAL)
		{
			// 无反应
			RELAY = 0;

		}
		// 寻车键
		if(control_signal == LOOK_FOR_SINAL)
		{
			speak_mode_set &= ~0x003f;
		//	state_current = UN_LOCK_STATE;
			// 寻车的铃声
			speak_mode_set |= 0x0002;
			speak_mode = lookformode;
			init_speak_param();
			// wait_for_alert();
		}
		// 设定键
		if(control_signal == LOCK_SIGNAL)
		{
			speak_mode_set &= ~0x003f;
			state_current = LOCK_STATE;
			first_alert = 0;
    		BACK = 0;
			speak_mode_set |= 0x0004;
			speak_mode = firstalertmode;
			init_speak_param();

		}
		// 解锁键
		if(control_signal == UNLOCK_SIGNAL)
		{
			speak_mode_set &= ~0x003f;
			state_current = UN_LOCK_STATE;
			speak_mode_set |= 0x0020;
			speak_mode = buzzer;
			init_speak_param();
		}

		if(control_signal == KEY_SIGNAL)
		{
			time_init();
			reset_time_counter();
			reset_speed_counter();
			mileage_init();
			BACK = 1;
			RELAY = 1;
			state_current = KEY_NORMAL_STATE;
		}

	}

	else if(state_current_pre == ALERT_STATE){
		// 正常键：取消报警
		if(control_signal == NORMAL_SIGNAL)
		{
			//关闭喇叭
			RELAY = 0;
			R_LIHGT = 0;
			L_LIHGT = 0;
			speak_mode_set &= ~0x003f;
			state_current = LOCK_STATE;
			enter_lock_state_15s = NO;
            reset_10s_counter();
         	first_alert = 0;
			wait_for_alert();
		}
		// 震动信号
		if(control_signal == SHOCK_SIGNAL)
		{
		}
		// 寻车键
		if(control_signal == LOOK_FOR_SINAL)
		{
			speak_mode_set &= ~0x003f;
			// state_current = UN_LOCK_STATE;
			// 寻车的铃声
			speak_mode_set |= 0x0002;
			speak_mode = lookformode;
			init_speak_param();
			// wait_for_alert();
		}
		// 设定键
		if(control_signal == LOCK_SIGNAL)
		{
			speak_mode_set &= ~0x003f;
//			wait_for_alert();
			state_current = LOCK_STATE;
			first_alert = 0;
    		BACK = 0;
			speak_mode_set |= 0x0004;
			speak_mode = firstalertmode;
			init_speak_param();
		}

		// 解锁键：不响应
		if(control_signal == UNLOCK_SIGNAL)
		{
			speak_mode_set &= ~0x003f;
			state_current = UN_LOCK_STATE;
			speak_mode_set |= 0x0020;
			speak_mode = buzzer;
			init_speak_param();
		}

		if(control_signal == KEY_SIGNAL)
		{
			speak_mode_set &= ~0x003f;
			time_init();
			reset_time_counter();
			reset_speed_counter();
			mileage_init();
			BACK = 1;
			RELAY = 1;
			state_current = KEY_NORMAL_STATE;
		}
	}

	else if (state_current_pre == NORMAL_STATE){
		// 解锁键
		if(control_signal == NORMAL_SIGNAL)
		{ 

		}
		// 震动信号
		if(control_signal == SHOCK_SIGNAL)
		{
			//无反应
		}
		// 寻车键
		if(control_signal == LOOK_FOR_SINAL)
		{
			speak_mode_set &= ~0x003f;
			speak_mode_set |= 0x0002;
			speak_mode = lookformode;
			init_speak_param();
			// wait_for_alert();
		}
		// 设定键
		if(control_signal == LOCK_SIGNAL && !speed_counter)
		{
			Store_mileage();
			speak_mode_set &= ~0x0fff; //关闭所有声音
			state_current = LOCK_STATE;
			first_alert = 0;
    		BACK = 0;
			speak_mode_set |= 0x0004;
			speak_mode = firstalertmode;
			init_speak_param();
			wait_for_alert();
		}

		// 不响应解锁键
		if(control_signal == UNLOCK_SIGNAL)
		{
			// speak_mode_set &= ~0x003f;
			// state_current = UN_LOCK_STATE;
			// speak_mode_set |= 0x0001;
			// speak_mode = unlockmode;
			// init_speak_param();
		}

		if(control_signal == KEY_SIGNAL)
			state_current = KEY_NORMAL_STATE;
	}
}


void read_control_signal()
{
	remote_signal = 0;
	remote_signal |= SIGNAL_D3;
	remote_signal <<= 1;
	remote_signal |= SIGNAL_D2;
	remote_signal <<= 1;
	remote_signal |= SIGNAL_D1;
	remote_signal <<= 1;
	remote_signal |= SIGNAL_D0;
}

// 信号处理
void handle_control_signal()
{
	state_enter_trans(remote_signal);
}




//控制信号中断(上升沿中断)
//中断1
void exint1() interrupt 2       //INT1中断入口
{
    if(INT1)
    {
    	read_control_signal();
		INT_PROC |= SIGNAL_RECEIVE_DUTY;
    } 
}