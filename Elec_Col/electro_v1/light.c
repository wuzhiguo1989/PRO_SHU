#include "light.h"
#include "jl17467pqw.h"
#include "share.h"
#include "beep.h"
#include "utils.h"
#include "signal.h"


void light_io_init()
{
	push_pull(2, 0x0F);
}

void light_init()
{
	STOP_LIGHT = 0;
	R_LIHGT = 0;
	L_LIHGT = 0;
	BACK = 0;
	RELAY = 0;
}



void light_follow_speak()
{
	if(state_current != NORMAL_STATE)
	{
		if(MUTE)
		{
		//	RELAY = 1;
			RELAY = 0;
			R_LIHGT = 1;
			L_LIHGT = 1;
		}
		else
		{
			RELAY = 0;
			R_LIHGT = 0;
			L_LIHGT = 0;
		}
		BACK = 0;
		display_clean();
	}
	else 
	{
		BACK = 1;
		RELAY = 1;
	}
}


void key_scan_task()
{
	if(!L_KEY)
	{
		jt17467pqw_set_left_lamps(time_colon_icon);
		L_LIHGT = time_colon_icon;
		// 喇叭优先级高
		if(speak_mode_set & 0x0100)
		{
			speak_mode = buzzer;
			MUTE = 1;
		}
		else
		{
			speak_mode_set |= 0x40;
        	speak_mode = light_alert;
        	MUTE = time_colon_icon;
        }
	}
	else
	{
        jt17467pqw_set_left_lamps(0);
		L_LIHGT = 0;
		speak_mode_set &= ~0x40;
	}

	if(!R_KEY)
	{
		jt17467pqw_set_right_lamps(time_colon_icon);
		R_LIHGT = time_colon_icon;
		if(speak_mode_set & 0x0100)
		{
			speak_mode = buzzer;
			MUTE = 1;
		}
		else
		{
			speak_mode_set |= 0x80;
        	speak_mode = light_alert;
        	MUTE = time_colon_icon;
        }

	}
	else
	{

        jt17467pqw_set_right_lamps(0);
		R_LIHGT = 0;
		speak_mode_set &= ~0x80;
	}

	jt17467pqw_set_head_lamps(!D_KEY);
	jt17467pqw_set_bluetooth_linked(BT_IND);
}
