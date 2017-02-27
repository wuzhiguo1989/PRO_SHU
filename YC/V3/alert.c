#include "adc.h"
#include "alert.h"
#include "task.h"
#include "dwscreen.h"
#include "process.h"
#include "workflow.h"

u8 alert_for_system = 0;
u8 record = 0;



static void init_io_feed_lack()
{
	//P23
	pure_input(2, 0x08);
}


void init_io_alert()
{
	init_io_feed_lack();
	init_adc();
	// P24|23
	push_pull(2, 0x18);
}

void alert_for_overload()
{
	//TODO
	if (!OVER_LOAD_IO)
		alert_for_system &= ~ALTER_OVER_LOAD;
  	else
		alert_for_system |= ALTER_OVER_LOAD;
}


u8 tem_around_pre = 0;
u8 tem_fan_pre = 0;

void send_alter_to_screen(u8 value)
{
	TX_write2buff(COM[COM2_ID], 0x5A);
	TX_write2buff(COM[COM2_ID], 0xA5);
	TX_write2buff(COM[COM2_ID], 0x05);
	TX_write2buff(COM[COM2_ID], 0x82);
	TX_write2buff(COM[COM2_ID], 0);
	TX_write2buff(COM[COM2_ID], 1);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], value);
}

void alert_for_temperature()
{
	tem_around = temperature_around_cal();
    tem_fan = temperature_fan_cal();
    if(tem_around_pre != tem_around || tem_fan_pre != tem_fan)
    	send_two_value_to_screen(ADDR_TEMPERATURE_AROUND, tem_around, tem_fan);
	tem_around_pre = tem_around; 
    tem_fan_pre = tem_fan;
    if(tem_fan >= tem_stop){
    	alert_for_system |= ALTER_TEM_STOP;
    	alert_for_system |= ALTER_TEM_BEEP;
    } else if (tem_fan >= tem_alter && tem_fan < tem_stop){
    	alert_for_system |= ALTER_TEM_BEEP;
    	alert_for_system &= ~ALTER_TEM_STOP;
    } else {
    	alert_for_system &= ~ALTER_TEM_STOP;
    	alert_for_system &= ~ALTER_TEM_BEEP;
    }
}


void get_RTC_from_screen()
{
	TX_write2buff(COM[COM2_ID], 0x5A);
	TX_write2buff(COM[COM2_ID], 0xA5);
	TX_write2buff(COM[COM2_ID], 0x03);
	TX_write2buff(COM[COM2_ID], 0x81);
	TX_write2buff(COM[COM2_ID], 0x20);
	TX_write2buff(COM[COM2_ID], 0x07);
}


// 报警解除标志
u8 alert_for_system_pre = 0;
void alert_beep()
{
	u8 alter_change_signal = 0;
	// 减少串口工作量
	alter_change_signal = alert_for_system_pre  ^ alert_for_system;
	record = alter_change_signal & alert_for_system;
					
	// 报警值变化
	if(alter_change_signal)
	{

		if(alter_change_signal & 0x0E)
			send_alter_to_screen(alert_for_system);
		// 报警开始
		if(alert_for_system)
		{
			if(record)
			// 记录时间
				get_RTC_from_screen();
			if(alert_for_system & 0x15)
				INT_PROC |= SYSTEM_END_DUTY;
			if(alert_for_system & 0x1E)
				INT_PROC |= ALTER_BEEP_DUTY;
		}
		// 报警解除
		else
			INT_PROC |= ALTER_BEEP_CLOSE_DUTY;
	}

	alert_for_system_pre = alert_for_system;
}