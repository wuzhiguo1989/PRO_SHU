#ifndef __SIGNAL_H
#define __SIGNAL_H

#include "stc15wxxxx.h"
#define RELAY  P23
#define KEY_ON    P00
#define SIGNAL_D0 P16
#define SIGNAL_D1 P17
#define SIGNAL_D2 P54
#define SIGNAL_D3 P55
#define SHOCK_SIGNAL   0x10
#define NORMAL_SIGNAL  0x04
#define LOCK_SIGNAL    0x08
#define LOOK_FOR_SINAL 0x02
#define UNLOCK_SIGNAL  0x01
#define KEY_SIGNAL     0x00


void init_control_signal();
void read_control_signal();
void state_enter_trans(u8 control_signal);
void handle_control_signal();
void wait_for_alert();
//void key_on_io_init();
void key_on_scan();

#endif