# include "alarm.h"
# include "utils.h"
static void delay_alarm_220V() // 300ms
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 28;
	j = 92;
	k = 196;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}


static void delay_alarm_batter() // 100ms
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 10;
	j = 31;
	k = 147;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}


void alarm_io_init()
{
	push_pull(5, 0x01);
}

void alarm_for_batter_low()
{
	ALARM = 1;
	delay_alarm_batter();
	ALARM = 0;
	delay_alarm_batter();
}


void alarm_for_220v_lost()
{
	ALARM = 1;
	delay_alarm_220V();
	ALARM = 0;
	delay_alarm_220V();
}

void alarm_for_key_cabinet()
{
	ALARM = 1;
}
