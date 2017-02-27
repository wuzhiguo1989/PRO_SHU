#ifndef __LIGHT_H_
#define __LIGHT_H_

#include "stc15wxxxx.h"

#define R_KEY  P01
#define L_KEY  P03
#define D_KEY  P02
#define BT_IND P14
#define BACK   P27


#define STOP_LIGHT   P20
#define R_LIHGT      P22
#define L_LIHGT      P21


void light_io_init();
void light_init();
void light_follow_speak();
void key_scan_task();

#endif