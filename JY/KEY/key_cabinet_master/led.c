#include "led.h"
#include "share.h"
#include "utils.h"
#include "alarm.h"

static code u8 const displaycode[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
									/*    0,    1,    2,    3,    4,    5,    6,    7,    8,    9   */

// static code u8 displaycode[10] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};
							/*    0,    1,    2,    3,    4,    5,    6,    7,    8,    9   */

void display_io_init()
{
    push_pull(5, 0x08);
    push_pull(0, 0x20);
    push_pull(0, 0x40);
    push_pull(0, 0x80);
    push_pull(6, 0x01);
    push_pull(6, 0x02);
    push_pull(6, 0x04);
    push_pull(6, 0x08);
    push_pull(4, 0x80);
}


static void delay_display() // 1ms
{
	unsigned char i, j;
	i = 24;
	j = 85;
	do
	{
		while (--j);
	} while (--i);
}

static void display_seg(u8 i, u8 number)
{
	if(i == 0){
		DISPLAY_D(0) = number;
	}else if(i == 1){
		DISPLAY_D(1) = number;
	}else if(i == 2){
		DISPLAY_D(2) = number;
	}else if(i == 3){
		DISPLAY_D(3) = number;
	}else if(i == 4){
		DISPLAY_D(4) = number;
	}else if(i == 5){
		DISPLAY_D(5) = number;
	}else if(i == 6){
		DISPLAY_D(6) = number;
	}
}

void display_number(u8 number)
{
	u8 i, code_store, code_buffer;
	code_buffer = displaycode[number];
	for(i = 0; i < 7; i++)
	{
		code_store = code_buffer;
		code_buffer = code_buffer >> 1;
		display_seg(i, (code_store - (code_buffer << 1)));
	}
}


void display_error(u8 number)
{
	DISPLAY_DG2 = 1;
	display_number(number/10);
	DISPLAY_DG1 = 0;
	delay_display();
	DISPLAY_DG1 = 1;
	display_number(number%10);
	DISPLAY_DG2 = 0;
	delay_display();
}

void no_error_display()
{
    DISPLAY_DG1 = 1;
    DISPLAY_DG2 = 1;
}


void led_display()
{
    if(!number_for_badkey)
    {
        performance_status &= ~0x20;
        no_error_display();
        if(performance_status == 0x00)
        	alarm_for_no_problem();
    }
    else
    {
        performance_status |= 0x20;
        display_error(order_for_badkey[0]);
        alarm_for_key_cabinet();
    }
}