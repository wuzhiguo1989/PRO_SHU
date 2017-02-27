#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include "config.h"
#include "led.h"

#define ADC_POWER   0x80            //ADC电源控制位
#define ADC_FLAG    0x10            //ADC完成标志
#define ADC_START   0x08            //ADC起始控制位
#define ADC_SPEEDLL 0x00            //540个时钟
#define ADC_SPEEDL  0x20            //360个时钟
#define ADC_SPEEDH  0x40            //180个时钟
#define ADC_SPEEDHH 0x60            //90个时钟
#define ADC_SPEED 	ADC_SPEEDHH
#define	ADRJ	0x20	//B5 [B7:B0]

typedef void (code *display_driver_pt) (void);
typedef void (code *driver_init_pt) (void);
typedef void (code *half_cycle_reached_pt) (void);

// T is comment with speed_level = 5 (UI display = 50)
typedef enum display_mode_type {
	// Stay Light OFF
	led_display_off = 0,

	moon_light,

	fire_work,

	hot_wheels,

	rainbow_chasing,

	color_chasing_7,

	color_breathe_3,

	color_breathe_7,

	white_breathe,

	combination,

	music_sensor
} disp_t;

typedef unsigned char signal_t;

typedef enum control_signal_type {
	// Switch On | Off 
	switch_on_off = 0,
	// Display Mode Select
	switch_display_mode,
	// Luminous value in [0, 10]
	adjust_lumins_level,
	// Speed value in [0, 10]
	adjust_speed_level
} ctrl_sig_t;

typedef bool toggle_t;
typedef unsigned char speed_level_t;
extern  volatile disp_t led_mode_set;
extern  volatile disp_t led_mode_logic;
extern  volatile speed_level_t speed;

#define LUMINS_LEVEL_RAW_MAX 0x10
#define SPEED_LEVEL_RAW_MAX 0x10

// recieved toggle status x == 0x01 || x == 0x02
#define check_toggle_status(x) (x == OFF + 1 || x == ON + 1)
// recieved display mode x <= senser_mode(12)
#define check_display_mode_type(x) (!(x > music_sensor))
// recieved lumins x > 0 && x <= LUMINS_LEVEL_RAW_MAX
#define check_lumins_level(x) (x && !(x > LUMINS_LEVEL_RAW_MAX))
// recieved speed x > 0 && x <= SPEED_LEVEL_RAW_MAX
#define check_speed_level(x)  (!(x > SPEED_LEVEL_RAW_MAX))

// public declaring
void display_startup_init();								// called by main_isr
void led_display_task();

#define LED_MODE_LOGIC_CHG	0
#define LED_MODE_SET_CHG	1
void led_mode_changed();


#define RISE 0
#define FALL 1

#define FULL 1
#define HALF 0

#define ON 1
#define OFF 0

#define COMBINATION_FIRST 2
#define COMBINATION_LAST 9

// x is belong to left-closed, right-open interval
#define belong_to_interval_lcro(x, left, right) (x >= left && x < right)
// x is belong to left-closed interval
#define belong_to_interval_lc(x, left) (x >= left)

void led_syc_color(u8 *param);
void ctrl_mode_changed(const u8 mode_code);
void ctrl_lumins_changed(const u8 lumins);
void ctrl_speed_level_changed(const u8 speed);
void init_adc();


#endif