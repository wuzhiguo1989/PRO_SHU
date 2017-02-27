#ifndef __TASK_H
#define __TASK_H

#include "config.h"
#include "typealias.h"
#include "dwscreen.h"

#define NUMBER_BUNKER  2

#define DW_DUTY                0x0001
#define WF_FINISH_DUTY         0x0002
#define WF_FLOW_DUTY           0x0004
#define COM2_RX_DUTY           0x0008
#define S_WF_DUTY              0x0010
#define TICK_DUTY			   0x0020
#define ALTER_FOR_FEED_LACK_DUTY 0x0040
#define SECOND_DUTY			   0x0080
#define START_DUTY			   0x0100
#define SYSTEM_END_DUTY        0x0200
#define SPRAY_WASH_DUTY        0x0400
#define FEED_SUCTION_END_DUTY  0x0800
#define ALTER_FOR_FEED_FULL_DUTY 0x1000
#define ALTER_BEEP_DUTY        0x2000
#define ALTER_BEEP_CLOSE_DUTY  0x4000
#define ALTER_FULL_FEED_CANCLE_DUTY 0x8000

#define COUNTER_PER_100MS       10

extern volatile u16 xdata INT_PROC;
extern volatile bool start_fan_signal;
// 放料过程料位时间计时标记位
extern volatile bool signal_detect_enable;
// 延时开机计时状态位
extern volatile bool signal_feed_full_delay;
// 放料确认完成
extern volatile bool signal_feed_lack_keep;

typedef void (code *event_cb) (void);

typedef struct count_down_entity {
	u8 second;
	u8 millisecond;
	event_cb p_callback;
} count_down_t;

extern count_down_t xdata fan_start_deceted;
extern count_down_t xdata feed_suction[NUMBER_BUNKER];
extern count_down_t xdata feed_discharge[NUMBER_BUNKER];
extern count_down_t xdata feed_intercept[NUMBER_BUNKER];
extern count_down_t xdata spray_wash[NUMBER_BUNKER];
extern count_down_t xdata spray_wash_between[NUMBER_BUNKER];
extern count_down_t xdata spray_wash_stop[NUMBER_BUNKER];


// test
extern count_down_t xdata test_spray_wash[NUMBER_BUNKER];
extern count_down_t xdata test_suction[NUMBER_BUNKER];
extern count_down_t xdata test_intercept[NUMBER_BUNKER];
extern count_down_t xdata test_fan;
extern count_down_t xdata test_beep;

// system
extern count_down_t xdata system_back_home;
extern count_down_t xdata password_no_identified;

void system_tick_init();
void Extern0_Init(void);
void call_for_duty(u16 duty_code);
bool working_for_duty(u16 duty_code);
void count_down_init(count_down_t xdata *cnt, event_cb func);
void count_down_reload(count_down_t xdata *cnt, const u8 second, const u8 millisecond); 
void count_down_all_task();
void count_down_cancel(count_down_t xdata *cnt);
void count_down_cancle_all_task();
void count_down_init_all_task();


void count_down_cancel_test();


// work
#define reload_fan_start_deceted() count_down_reload(&fan_start_deceted, 1, 0)
void reload_feed_suction(u8 local_nmber);
void reload_feed_discharge(u8 local_nmber);
void reload_feed_intercept(u8 local_nmber);
// #define reload_spray_wash_start() count_down_reload(&spray_wash_start, 2, 0)
void reload_spray_wash(u8 local_nmber);
void reload_spray_wash_between(u8 local_nmber);
void reload_spray_wash_stop(u8 local_nmber);

// test
void reload_test_spray_wash(u8 local_nmber);
void reload_test_suction(u8 local_nmber);
void reload_test_intercept(u8 local_nmber);
#define reload_test_fan() count_down_reload(&test_fan, 10, 0)
#define reload_test_beep() count_down_reload(&test_beep, 10, 0)

// system
#define reload_system_back_home() count_down_reload(&system_back_home, 180, 0)
#define reload_system_no_identify() count_down_reload(&password_no_identified, 180, 0)


#define FAN_VALVE			 	 P03
#define SPRAY_WASH_VALVE[1]      P01
#define SPRAY_WASH_VALVE[2]
#define SUCTION_DISCHARGE_VALVE[1]  P02
#define SUCTION_DISCHARGE_VALVE[2]  
#define INTERCEPT_VALVE[1]		P00
#define INTERCEPT_VALVE[2]		

void spray_wash_enable(u8 local_nmber);
void spray_wash_disable(u8 local_nmber);
void suction_enable(u8 local_nmber);
void discharge_enable(u8 local_nmber);
void intercept_enable(u8 local_nmber);
void intercept_disable(u8 local_nmber);
#define fan_work_enable()     FAN_VALVE = 1
#define fan_work_disable()    FAN_VALVE = 0

void system_start_or_not();
void valve_disable();
void valve_control_io_init();
void detected_feed_full_start();
void feed_discharge_task();
void start_spray_wash();
void feed_suction_callback();

// flag set to 0
// clear set to 1
#define  WDT_FLAG	0x80
#define  CLR_WDT	0x10

#define reset_watch_dog()     WDT_CONTR &= ~WDT_FLAG;	WDT_CONTR |= CLR_WDT

#endif