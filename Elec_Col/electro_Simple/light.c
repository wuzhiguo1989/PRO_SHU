#include "light.h"
#include "jl17467pqw.h"
#include "share.h"
#include "beep.h"
#include "utils.h"

void init_key_io()
{
	pure_input(1,0x20);
	pure_input(1,0x10);
	pure_input(5,0x10);
	pure_input(5,0x20);
}

void key_scan_task()
{
	// 	if (!BUZZER)
//     {
//         mute_beep_enable();
//         speak_mode_set |= 0x0100;
//         speak_mode = buzzer;
        
//     }
//     else
//         speak_mode_set &= ~0x0100;


	if(L_KEY)
	{
		jt17467pqw_set_left_lamps(1);
		speak_mode_set |= 0x01;
    speak_mode = light_alert;
    MUTE = 1;
	}
	else
	{
        jt17467pqw_set_left_lamps(0);
		speak_mode_set &= ~0x01;
	}

	if(R_KEY)
	{
		jt17467pqw_set_right_lamps(1);
		speak_mode_set |= 0x02;
        speak_mode = light_alert;
       MUTE = 1;
	}
	else
	{

        jt17467pqw_set_right_lamps(0);
		speak_mode_set &= ~0x02;
	}

	jt17467pqw_set_head_lamps(D_KEY);
	jt17467pqw_set_bluetooth_linked(BT_IND);
}
