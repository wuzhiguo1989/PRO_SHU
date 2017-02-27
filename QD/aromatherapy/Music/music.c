#include "music.h"
#include "utils.h"
#include "ble.h"

volatile music_t xdata music_config_set;
volatile music_status_t xdata music_status = 0;

//                                 0,     1,   0    1     0    1     0    1     0     1      0
u16 music_volume_signal[13] = {0x8300, 0xF830, 0, 0xFC18, 0, 0xFC18, 0, 0xFC18, 0, 0xFC18, 0, 0xFC18, 0xFC18};

u8 volume_config_start = 0;
u8 volume_work_on = 0;

//                                 0,      1,     0      1       0      1      0      1      0     1
u16 music_control_signal[13] ={0x8300, 0xF830, 0, 0xFC18, 0, 0xFC18, 0, 0xFC18, 0, 0xFC18, 0, 0xFC18, 0xFC18};
u8 control_config_start = 0;
u8 control_work_on = 0;

void init_music_cotrol() {
    Timer1_12T();
    Timer1_AsTimer();
    Timer1_16bitAutoReload();
    T1_Load(0xF830);
    Timer1_InterruptEnable();
    Timer1_Stop();
    // Timer1_Run();
    push_pull(1, 0x10); // 歌曲
	push_pull(1, 0x08); // 声音
	push_pull(1, 0x02); // MUTE
	MUTE = 0;
}

// 选曲接收：收到对应码执行对应功能。
// 数据码：0001，单曲循环第1首语音
// 		 0010，单曲循环第2首语音
// 		 0011，单曲循环第3首语音
// 		 0100，单曲循环第4首语音
// 		 0101，单曲循环第5首语音
// 		 0110，单曲循环第6首语音
// 		 0111，暂停播放
// 		 1000，播放语音
// 		 1001，上一曲
// 		 1010，下一曲
// 		 1111，全部循环，6首语音循环播放，每首播放一次

// 音量接收：收到对应码切换对应音量，总共16级音量，最小级为静音（并关闭功放）。
// 数据码：0000，第1级音量
// 0001，第2级音量
// 		 0010，第3级音量
// 		 0011，第4级音量
// 		 0100，第5级音量
// 		 0101，第6级音量
// 		 0110，第7级音量
// 		 0111，第8级音量
// 		 1000，第9级音量
// 		 1001，第10级音量
// 		 1010，第11级音量
// 		 1011，第12级音量
// 		 1100，第13级音量
// 		 1101，第14级音量
// 		 1110，第15级音量
// 		 1111，第16级音量


void music_select_10bit()
{
	u8 i, from = 2;
	u8 odd = 0;
	u8 data_mask = 0x08;
	music_select_t music_select_value = music_config_set.music_select;
	for(i = 0; i < 4; i++)
	{
		if(music_select_value & data_mask)
		{
			odd++;
			music_control_signal[from] = 0xF448;
		}
		else
			music_control_signal[from] = 0xFC18;
		from += 2;
		data_mask >>= 1;
	}
	if(!(odd % 2))
		music_control_signal[from] = 0xFC18;
	else
		music_control_signal[from] = 0xF448;
}

void music_control_config()
{
	music_select_10bit();
	control_config_start = 0;
	control_work_on = 1;
	T1_Load(music_control_signal[control_config_start]);
	Timer1_Run();
	SONG_SELECT = 0;	
}

void music_volume_10bit()
{
	u8 i, from = 2;
	u8 odd = 0;
	u8 data_mask = 0x08;
	music_select_t music_volume_value = music_config_set.music_volume;
	for(i = 0; i < 4; i++)
	{
		if(music_volume_value & data_mask)
		{
			music_volume_signal[from] = 0xF448;
			odd++;
		}
		else
			music_volume_signal[from] = 0xFC18;
		from += 2;
		data_mask >>= 1;

	}
	if(!(odd % 2))
		music_volume_signal[from] = 0xFC18;
	else
		music_volume_signal[from] = 0xF448;

}

void music_volume_config()
{
	music_volume_10bit();
	volume_config_start = 0;
	volume_work_on = 1;
	VOLUME = 0;
	T1_Load(music_volume_signal[volume_config_start]);
	Timer1_Run();	
}

// void Delay16ms()		//@24.000MHz
// {
// 	unsigned char i, j, k;

// 	_nop_();
// 	_nop_();
// 	i = 2;
// 	j = 118;
// 	k = 132;
// 	do
// 	{
// 		do
// 		{
// 			while (--k);
// 		} while (--j);
// 	} while (--i);
// }

// void Delay1ms()		//@24.000MHz
// {
// 	unsigned char i, j;

// 	i = 24;
// 	j = 85;
// 	do
// 	{
// 		while (--j);
// 	} while (--i);
// }

// void Delay500us()		//@24.000MHz
// {
// 	unsigned char i, j;

// 	i = 12;
// 	j = 169;
// 	do
// 	{
// 		while (--j);
// 	} while (--i);
// }

// void Delay1500us()		//@24.000MHz
// {
// 	unsigned char i, j;

// 	i = 36;
// 	j = 1;
// 	do
// 	{
// 		while (--j);
// 	} while (--i);
// }

// void select_song()
// {
// 	SONG_SELECT = 0;
// 	Delay16ms();
// 	SONG_SELECT = 1;
// 	Delay1ms();
// 	// 0
// 	SONG_SELECT = 0;
// 	Delay500us();
// 	SONG_SELECT = 1;
// 	Delay500us();

// 	// 0
// 	SONG_SELECT = 0;
// 	Delay500us();
// 	SONG_SELECT = 1;
// 	Delay500us();

// 	// 0
// 	SONG_SELECT = 0;
// 	Delay500us();
// 	SONG_SELECT = 1;
// 	Delay500us();

// 	// 1
// 	SONG_SELECT = 0;
// 	Delay1500us();
// 	SONG_SELECT = 1;
// 	Delay500us();

// 	SONG_SELECT = 0;
// 	Delay1500us();
// 	SONG_SELECT = 1;
// 	Delay500us();

// 	//结束符
// 	SONG_SELECT = 0;
// 	Delay500us();

// 	// 置高
// 	SONG_SELECT = 1;
// }

void music_config_init()
{
	if(!music_status)
	{
		music_config_set.music_select = 1;
		music_control_config();
		music_status = 1;
	}
}

void music_select_songs(music_select_t select_number)
{
	music_config_set.music_select = select_number;
	music_control_config();
}

void music_select_volume(music_volume_t *volume_number)
{
	if(volume_number[5] == '\n')
		music_config_set.music_volume = volume_number[4] - '0';
	else
		music_config_set.music_volume = (volume_number[4] - '0') * 10 + (volume_number[5] - '0');
	music_volume_config();
}

void music_switch_on_off(music_status_t status_on)
{
	if(!status_on)
	{
		music_status = OFF;
		MUTE = 0;
	}
	else
	{
		music_status = ON;
		MUTE = 1;
	}

}




void tm1_isr() interrupt TIMER1_VECTOR  using 2
{
	if(control_work_on)
	{
		control_config_start ++;
		Timer1_Stop();
		if(control_config_start == 13)
		{
			control_config_start = 0;
			control_work_on = 0;
		}
		else
		{
			T1_Load(music_control_signal[control_config_start]);
			Timer1_Run();
		}
		SONG_SELECT = ~SONG_SELECT;
	}

	if(volume_work_on)
	{
		volume_config_start ++;
		Timer1_Stop();
		if(volume_config_start == 13)
		{
			volume_config_start = 0;
			volume_work_on = 0;
		}
		else
		{
			T1_Load(music_volume_signal[volume_config_start]);
			Timer1_Run();
		}
		VOLUME = ~VOLUME;
	}
	
}



