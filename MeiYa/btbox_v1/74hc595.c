# include "74hc595.h"
# include "share.h"
# include "sync.h"
# include "utils.h"
# include "task.h"

#define NOT_DISPLAY  10

#define TIME_ICON  0x14

static code u8 const displaycode[12] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x00, 0x79};
								   /*    0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   NOT_DISPLAY */
static code u8 const comcode[6] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20};
								/*  1,    2,    3,    4,    5,    6    */
static code u8 const power[6] = {0x00, 0x01, 0x03, 0x0B, 0x2B, 0x6B};
							/*  1,    2,    3,    4,    5,    6    */
static code u8 const color[25] = {0x01, 0x05, 0x15, 0x11, 0x0D, 0x1D, 0x31, 0x35, 0x3D, 0x03, 0x07, 0x17, 0x13, 0x0F, 0x1F, 0x3F, 0x33, 0x37, 0x04, 0x14, 0x34, 0x0C, 0x1C, 0x10, 0x30};

u8 LED8[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

u8 display_index = 0;


void Led_io_init(void)
{
	push_pull(1, 0x02);
	push_pull(1, 0x01);
	push_pull(2, 0x80);
	push_pull(2, 0x40);
	push_pull(2, 0x20);
	push_pull(2, 0x10);
	push_pull(2, 0x04);
	push_pull(2, 0x02);
	push_pull(2, 0x01);
	push_pull(3, 0x80);
	push_pull(3, 0x40);
	push_pull(3, 0x20);
	push_pull(3, 0x10);	
}


void Delay3000ms()		//@24.000MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 125;
	j = 250;
	k = 250;
	do
	{
		do
		{
			while (--k);
		} while (--j);
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


void display_number(u8 dat)
{       
    u8  i;
    for(i = 0; i < 7; i++)
    {
        if(dat & 0x01)
        	display_seg(i, 1);
        else
        	display_seg(i, 0);
        dat >>= 1;
    }
}

static void display_com(u8 i, u8 number)
{
	if(i == 0){
		DISPLAY_C(0) = number;
	}else if(i == 1){
		DISPLAY_C(1) = number;
	}else if(i == 2){
		DISPLAY_C(2) = number;
	}else if(i == 3){
		DISPLAY_C(3) = number;
	}else if(i == 4){
		DISPLAY_C(4) = number;
	}else if(i == 5){
		DISPLAY_C(5) = number;
	}
}

void display_com_number(u8 dat)
{       
    u8  i;
    for(i = 0; i < 6; i++)
    {
        if(dat & 0x01)
        	display_com(i, 1);
        else
        	display_com(i, 0);
        dat >>= 1;
    }
}




void Display_digit_on_screen(void)
{
   if (power_error_code == 2)
   {
   	// 分钟
    	LED8[0] = 10;
    	LED8[1] = 10;
    // 秒
    	LED8[3] = 11;
    	LED8[4] = 1;
    // 电源指示灯 + 时间
		LED8[2] = power_level;
    // 灯光
    	LED8[5] = color_level;
   }
   else
   {
   	// 分钟
    	LED8[0] = local_time.minute / 10;
    	LED8[1] = local_time.minute % 10;
    // 秒
    	LED8[3] = local_time.second / 10;
    	LED8[4] = local_time.second % 10;
    // 电源指示灯 + 时间
		LED8[2] = power_level;
    // 灯光
    	LED8[5] = color_level;
    }
}	

void DisplayScanInit(void)
{   
	display_com_number(~0x3F);
	display_number(0x7F);
    Delay3000ms();
}


void DisplayScan(void)
{   
//  先消隐	
	display_number(0x00);
    display_com_number(~comcode[display_index]); 
// 显示数据
    if(display_index == 0 || display_index == 1 || display_index == 3 || display_index == 4)
    {
    	if(!status_mb_key)
    	{
    		display_number(0x00);
    		time_colon_icon = 0;
    	} 
    	else
    		display_number(displaycode[LED8[display_index]]);   	//输出段码
    }  else if(display_index == 2) {
		if(!time_colon_icon || (power_error_code == 2))
			display_number(power[LED8[display_index]]);
		else
			display_number(TIME_ICON | power[LED8[display_index]]);   	
	} else {
		if(!status_mb_key)
		{
			display_number(0x00);
		} else if(status_mb_key == 1 || status_mb_key == 3){
			display_number(0x3F);
		} else if(status_mb_key == 2){
			display_number(color[LED8[display_index]]);
		}
	}

	display_com_number(~comcode[display_index]); 					//输出位码
    if(++display_index >= 6)
    	display_index = 0;  //8位结束回0
}
