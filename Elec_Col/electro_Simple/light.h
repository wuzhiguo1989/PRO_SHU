#ifndef __LIGHT_H_
#define __LIGHT_H_

#include "stc15wxxxx.h"

#define R_KEY  P54
#define L_KEY  P55
#define D_KEY  P15
#define BT_IND P14
void init_key_io();
void key_scan_task();

#endif