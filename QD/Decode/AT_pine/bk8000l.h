#ifndef __BK8000L_H_
#define __BK8000L_H_
#include "config.h"

#define COM_TX1_Lenth   64

#define TimeOutSet1     2

void Bk8000l_Init();

void Bk8000l_Sync_Year(u8 param);
void Bk8000l_Sync_Month(u8 param);
void Bk8000l_Sync_Day(u8 param);
void Bk8000l_Sync_Hour(u8 param);
void Bk8000l_Sync_Minute(u8 param);
void Bk8000l_Sync_Time(u8 year_param, u8 month_param, u8 day_param, u8 hour_param,u8 minute_param);

void Bk8000l_Change_DisplayMode(u8 param);
void Bk8000l_Change_DisplayLimus(u8 param);
void Bk8000l_Change_DisplaySpeed(u8 param);

void Bk8000l_Change_RedColor(u8 param);
void Bk8000l_Change_GreenColor(u8 param);
void Bk8000l_Change_BlueColor(u8 param);
void Bk8000l_Sync_Color(u8 red_param, u8 green_param, u8 blue_param);


void Bk8000l_Sync_ALARM1(u8 hour_param,u8 minute_param);
void Bk8000l_Sync_ALARM2(u8 hour_param,u8 minute_param);
void Bk8000l_Control_ALARM(u8 param);

#endif