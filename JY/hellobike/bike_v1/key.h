#ifndef __KEY_H_
#define __KEY_H_
#include "stc15wxxxx.h"


typedef void (code *driver_init_pt) (void);
typedef void (code *half_cycle_reached_pt) (void);


typedef enum display_mode_type {
	led_display_off = 0,
	schedule,
	trace,
	lock,
	open
} disp_t;

typedef u8 color_t;

typedef struct led_mode_entity{
	color_t red;
	color_t green;
	color_t blue;
} led_t;


typedef struct cycle_control_entity
{
	u8 cycle_output;
	u8 cycle_enable;
	u16 counter_for_cycle;
} cycle_control_t;


extern volatile cycle_control_t xdata led_flicker;
extern volatile disp_t xdata led_mode_set;



#define LED_DATA     P14
#define LED_POWER_1  P54
#define LED_POWER_2  P55
#define LED_CNT  4

#define led_power_enable()  LED_POWER_1 = 1; LED_POWER_2 = 1
#define led_power_disable() LED_POWER_1 = 0; LED_POWER_2 = 0


void led_io_init();
void led_config_change();
void display();

typedef void (code *event_cb) (void);

typedef struct count_down_entity {
	u16 second;
	u8 millisecond;
	event_cb p_callback;
} count_down_t;


void reload_cnt_for_beep();
void reload_lock_count_down();
void reload_open_count_down();
void reload_trace_beep_count_down();
void reload_trace_mute_count_down();
void reload_alter_beep_count_down();
void reload_sleep_count_down();
void reload_lock_sleep_count_down();
void reload_open_sleep_count_down();
void reload_alter_sleep_count_down();
void cancle_sleep_count_down();
void cancle_lock_count_down();
void cancle_open_count_down();
void reload_open_beep_count_down();
void reload_schedule_sleep_count_down();

void count_down_init_all_task();
void count_down_at_all_task();
void led_display_stop();




void init_sync_object();
void cycle_based_control();
void led_display_task();
void led_mode_changed();


#endif