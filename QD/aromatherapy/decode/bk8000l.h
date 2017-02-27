#ifndef __BK8000L_H_
#define __BK8000L_H_
#include "config.h"

#define COM_TX1_Lenth   64

#define TimeOutSet1     2

void Bk8000l_Init();

void TX1_write2buff(const u8 dat);

void Bk8000l_Change_DisplayMode(u8 param);
void Bk8000l_Change_Power(u8 param);
void Bk8000l_Change_DisplayLimus(u8 param);
void Bk8000l_Change_DisplaySpeed(u8 param);
void Bk8000l_Sync_Color(u8 red_param, u8 green_param, u8 blue_param);


void Bk8000l_Control_ALARM(u8 param);

#endif