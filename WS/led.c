#include "STC15Fxxxx.h" 
code unsigned char Data_left[10]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0xff,0x6f};
code unsigned char Data_right[10]={0xfc,0x60,0xda,0xf2,0x66,0xb6,0xbe,0xe0,0xff,0xf6};

#define BAUD    9600
#define FOSC    24000000L
#define T0INT   (65536-FOSC/8000)             // 8000次采样 / 秒
#define T2INT   (65536 - (FOSC/4/BAUD))       //


///  input
sbit P_RED = P3^3;
sbit P_YELLOW = P3^6;
sbit P_GREEN = P3^2;
//   output data
sbit MBI_SDI = P1^1;
sbit MBI_CLK = P1^0;
sbit MBI_OE = P5^4;
sbit MBI_LE = P3^7;

//   output	
sbit DISPLAY_RED = P1^2;
sbit DISPLAY_YELLOW = P1^3;
sbit DISPLAY_GREEN = P1^4;

 
//  
bit display_time_duty = 0; //显示时间任务标志位
bit green_display_duty = 0; //red -> green 
bit red_countdown_duty = 0; // red cnt dn
bit green_countdown_duty = 0; // green cnt dn
bit display_red_duty = 0;
bit display_green_duty = 0;
u8	green_interrupt = 1;
u8  red_interrupt = 0;
u8	counter_5ms = 200;   // 5ms定时计数
u8  counter_time = 0;	// 学习时间
u8  current_time = 0;  //  显示时间
u8  current_time_red = 0; // 红灯时间存储
u8	current_time_green = 0; // 绿灯时间存储

void Ext0_int()
{     
	IT0 = 0; 
	EX0 = 1;
	PX0 = 1;
}          
void Ext1_int()     
{
    IT1 = 0; 
    EX1 = 1;
    PX1 = 1;
} 
//========================================================================
// å‡½æ•°: void   Timer0_init(void)
// æè¿°: timer0åˆå§‹åŒ–å‡½æ•°.
// å‚æ•°: none.
// è¿”å›ž: none.
// ç‰ˆæœ¬: V1.0, 2015-1-12
//========================================================================
void Timer0_init(void)		//5毫秒@24.000MHz
{
	AUXR &= 0x7F;		//定时器时钟12T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0xF0;		//设置定时初值
	TH0 = 0xD8;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	ET0 = 0;
}

//-----------------------------------------
// display data
//-----------------------------------------
void Delay()		//@24.000MHz
{
//	unsigned char i;

	_nop_();
	_nop_();
//	i = 3;
//	while (--i);
}

void wr5024byte(unsigned char dat2,unsigned char dat1)
{
    unsigned char i;
		unsigned int dat;
		dat= dat2;
		dat=(dat<<8) | dat1;
		MBI_LE=0;
    for(i=0;i<16;i++)
    {
        MBI_SDI=(dat & 0x8000)?1:0;
        MBI_CLK=0;
//				Delay();
					_nop_();
        dat<<=1;
        MBI_CLK=1;
//				Delay();
					_nop_();
    }
    MBI_LE = 1;
//    Delay();
			_nop_();
    MBI_LE = 0;
//		Delay();
			_nop_();
    MBI_OE = 0;
}
void display_panel(unsigned char dat_panel)
{
   unsigned char split_data1,split_data0;
   split_data0=dat_panel%10;
   split_data1=dat_panel/10;
   wr5024byte(Data_left[split_data1],Data_right[split_data0]);
}
//-----------------------------------------
//ÖÐ¶Ï·þÎñ³ÌÐò
void exint0() interrupt 0       //INT0ÖÐ¶ÏÈë¿Ú
{
	green_interrupt ++;
	if(green_interrupt == 1)  // red ->　green
		green_display_duty = 1;
	if(green_interrupt == 2)
		green_countdown_duty = 1;
}
void exint1() interrupt 2       //INT0ÖÐ¶ÏÈë¿Ú
{	
	 red_interrupt ++;
	 if(red_interrupt == 3)  // red ->　green
		red_countdown_duty = 1;
}
//======================================================================
// å‡½æ•°: void main(void)
// æè¿°: ä¸»å‡½æ•°.
// å‚æ•°: none.
// è¿”å›ž: none.
// ç‰ˆæœ¬: V1.0, 2015-1-12
//======================================================================
void main(void)
{
	P1M0 = 0xFF;
	P1M1 = 0x00;
	P3M0 = 0x80;
	P3M1 = 0x7F;
	P5M0 = 0xFF;
	P5M1 = 0x00;
	Timer0_init();
	Ext0_int();
	Ext1_int();
	DISPLAY_RED = 0;
	DISPLAY_YELLOW = 0;
	DISPLAY_GREEN = 0;
	EA = 1;
	while(1)
	{
		if(P_YELLOW) //黄灯亮
		{
			DISPLAY_RED = 0;
			DISPLAY_YELLOW = 1;
			DISPLAY_GREEN = 0;
			display_time_duty = 0;
			if(green_interrupt)
			{	
				ET0 = 0;
				if(counter_5ms < 100)
					++counter_time;
				current_time_green = counter_time;
				counter_time = 0;
				counter_5ms = 200;
				green_interrupt = 0;
			}
//			continue;
		}
		if(!DISPLAY_RED && !DISPLAY_GREEN) //黄灯灭
		{
			DISPLAY_RED = 0;
			DISPLAY_YELLOW = 0;
			DISPLAY_GREEN = 0;
			display_time_duty = 0;
//			continue;
		}
		if(red_countdown_duty == 1) // red cnt dn
	 	{
			display_red_duty = 1;
			display_green_duty = 0;
//			DISPLAY_RED = 1;
			DISPLAY_YELLOW = 0;
			DISPLAY_GREEN = 0;
			display_time_duty = 1;
			current_time = current_time_red;

			ET0 = 1;
			red_countdown_duty = 0;
//			continue;
	 	}
		if(green_display_duty == 1) // red -> green
		{
			ET0 = 0;
			DISPLAY_RED = 0;
			DISPLAY_YELLOW = 0;
			DISPLAY_GREEN = 0;
			display_time_duty = 0;
			if(counter_5ms < 100)
					++counter_time;
			current_time_red = counter_time;
			counter_time = 0;
			counter_5ms = 200;
			red_interrupt = 0;
			green_display_duty = 0;
//			continue;
		}
		if(green_countdown_duty == 1) // green cnt dn
		{
			DISPLAY_RED = 0;
			DISPLAY_YELLOW = 0;
			display_green_duty = 1;
			display_red_duty = 0;
//			DISPLAY_GREEN = 1;
			display_time_duty = 1;	
			current_time = current_time_green;
			ET0 = 1;
			green_countdown_duty = 0;
//			continue;
		}
		if(display_time_duty)
		{
			if(current_time)
			{
				display_panel(current_time);
				if(display_red_duty)
				{
					DISPLAY_RED = 1;
					display_red_duty = 0;
				}
				if(display_green_duty)
				{
					DISPLAY_GREEN = 1;
					display_green_duty = 0;
				}

//			continue;
			}
		}

	}
}


//======================================================================
// å‡½æ•°: void timer0_int (void) interrupt TIMER0_VECTOR
// æè¿°:  timer0ä¸­æ–­å‡½æ•°.
// å‚æ•°: none.
// è¿”å›ž: none.
//======================================================================
void tm0_isr() interrupt 1 using 1
{
	-- counter_5ms;
	if(!counter_5ms) // 1s时间到
	{
		counter_time++; //学习计时
		if(!current_time)
		 	current_time = 1;
		--current_time; //显示时间
		counter_5ms = 200;
	}
}

