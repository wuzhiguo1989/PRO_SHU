#ifndef __KEY_H_
#define __KEY_H_

#include "config.h"

// BT
#define K1   P11
// POWER   
#define K2   P12
// LAMP
#define K3   P00
// FOG
#define K4   P27
// +
#define K5   P26
// -
#define K6   P25
//
#define K7   P33
// 
#define K8   P34
//
#define K9   P35

#define KEY_BT_ID    0x01
#define KEY_POWER_ID 0x02
#define KEY_LAMP_ID  0x04
#define KEY_FOG_ID   0x08
#define KEY_VA_ID    0x10
#define KEY_VM_ID    0x20
#define KEY_LIGHT_ID 0x40
#define KEY_BLUETOOTH_ID 0x80
#define KEY_MIST_ID  0x0100




#define LONG_KEY_TIME   100 // 	2s
#define SHORT_KEY_TIME  20

typedef void (code *event_cb) (void);

typedef struct count_down_entity {
	u8 second;
	u8 millisecond;
	event_cb p_callback;
} count_down_t;

extern count_down_t xdata volume_increase;
extern count_down_t xdata volume_decrease;
extern count_down_t xdata bluetooth_on_off;
extern count_down_t xdata bluetooth_pair;
// extern count_down_t xdata volume_increase_quick;
// extern count_down_t xdata volume_decrease_quick;

extern volatile u8 interval_volume_increase_hold;
extern volatile u8 interval_volume_decrease_hold;

void key_io_init();
void decrease_key_hold_interval();
u16 IO_KeyScan_G();
void key_scan_task();
void key_duty_task();
void count_down_init_all_task();
void count_down_at_all_task();

#endif