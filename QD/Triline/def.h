#ifndef __DEF_H_
#define __DEF_H_

#define EXINT_PROC 		0x01
#define TIMER_PROC 		0x02
#define UART_PROC		0x04
#define ADC_PROC		0x08
#define TICK_DUTY		0x10
#define MINUTE_DUTY		0x20
#define LM567_DUTY		0x40
#define RECIEVE_DUTY    0x80

typedef unsigned char u8;	// 0 - 255
typedef unsigned int u16;	// 0 - 65535
typedef unsigned long u32; 	// 0 - 4294967295
typedef bit BOOL;
typedef bit bool;

#define NO 0
#define YES 1
#define NULL (void*)0

#define true 1
#define false 0

#define ENABLE 1
#define DISABLE 0

// T is comment with speed_level = 5 (UI display = 50)
typedef enum display_mode_type {
	combination = 0,
	in_wave,
	sequential,
	slow_glo,
	chasing_flash,
	slow_fade,
	twinkle_flash,
	stacking_flash,
	flash_and_chase,
	random_flashing,
	pulsing,
	sequential_flashing,
	bi_directional_storbing,
	steady_on,
	sensor
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

typedef bit toggle_t;
typedef unsigned char lumins_level_t;
typedef unsigned char speed_level_t;

#define OFF 0
#define ON 1
#define switch_on(pin) high(pin)
#define switch_off(pin)	low(pin)

typedef unsigned char hertz_t;

#endif