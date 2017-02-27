#include "display.h"
#include "sync.h"
#include "utils.h"
#include "string.h"
#include "led.h"
#include "lm567.h"

volatile disp_t led_mode_set = 0;
volatile disp_t led_mode_logic = 0;
volatile speed_level_t speed = 9, led_logic_speed = 9;
volatile led_t xdata led_config_set[19], led_config_logic[19];
volatile toggle_t led_toggle_status = ON;

// -------------------- display config declaring -------------------------
// Cycle are calculated based on 5ms timer
static code u16 const cycle_for_led_display_off[16];
static code u16 const cycle_for_moonlight[16];
static code u16 const cycle_for_fire_work[16] = {
	160, 150, 140, 130, 120, 110, 100, 90, 80, 70, 60, 50, 40, 30, 20, 10
};
static code u16 const cycle_for_hot_wheels[16] = {
	160, 150, 140, 130, 120, 110, 100, 90, 80, 70, 60, 50, 40, 30, 20, 10
};
static code u16 const cycle_for_rainbow_chasing[16] = {
	320, 300, 280, 260, 240, 220, 200, 180, 160, 140, 120, 100, 80, 60, 40, 20
};
static code u16 const cycle_for_color_chasing_7[16] = {
	320, 300, 280, 260, 240, 220, 200, 180, 160, 140, 120, 100, 80, 60, 40, 20
};
static code u16 const cycle_for_color_breathe_3[16] = {
	32, 30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6, 4, 2
};
static code u16 const cycle_for_color_breathe_7[16] ={
	32, 30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6, 4, 2
};
static code u16 const cycle_for_white_breathe[16] ={
	32, 30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6, 4, 2
};
static code u16 const cycle_for_combination[16] ={
	320, 300, 280, 260, 240, 220, 200, 180, 160, 140, 120, 100, 80, 60, 40, 20
};
static code u16 const cycle_for_music_sensor[16] = {
	30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30
};


// ------------------------- marco functions -------------------------



// #define sync_up_toggle_stat()	toggle_stat_last_for_sensor_prevent_inst_blink = toggle_status

// #define LED_MODE_LOGIC_CHG	0
// #define LED_MODE_SET_CHG	1

// ------------------------- private declaring -------------------------
static void reload_pwm_config_if_should();
static void load_pwm_off_config();
static void prepare_next_time_led_config();
static void led_cycle_based_adjust();
static void half_cycle_reached();


// ------------------------- display_mode_init declaring -------------------------
static void led_display_off_driver_init();
static void moon_light_driver_init();
static void fire_work_driver_init();
static void hot_wheels_driver_init();
static void rainbow_chasing_driver_init();
static void color_chasing_7_driver_init();
static void color_breathe_3_driver_init();
static void color_breathe_7_driver_init();
static void white_breathe_driver_init();
static void combination_driver_init();
static void music_sensor_driver_init();

// ------------------------- display_mode_driver declaring -------------------------
static void led_display_off_driver();
static void moon_light_driver();
static void fire_work_driver();
static void hot_wheels_driver();
static void rainbow_chasing_driver();
static void color_chasing_7_driver();
static void color_breathe_3_driver();
static void color_breathe_7_driver();
static void white_breathe_driver();
static void combination_driver();
static void music_sensor_driver();
// ------------------------- half_cycle_reached declaring -------------------------

static void led_display_off_half_cycle_reached();
static void moon_light_half_cycle_reached();
static void fire_work_half_cycle_reached();
static void hot_wheels_half_cycle_reached();
static void rainbow_chasing_half_cycle_reached();
static void color_chasing_7_half_cycle_reached();
static void color_breathe_3_cycle_reached();
static void color_breathe_7_half_cycle_reached();
static void white_breathe_half_cycle_reached();
static void combination_half_cycle_reached();
static void music_sensor_half_cycle_reached();

static code const u16 *cycle_params[11] =
{
	NULL, 						// led_display_off
	NULL,						// moon_light
	cycle_for_fire_work, 					// fire_work
	cycle_for_hot_wheels,						// hot_wheels
	cycle_for_rainbow_chasing, 						// rainbow_chasing
	cycle_for_color_chasing_7,					// color_chasing_7
	cycle_for_color_breathe_3, 						// color_breathe_3
	cycle_for_color_breathe_7,						// color_breathe_7
	cycle_for_white_breathe, 						// white_breathe
	cycle_for_combination,						// combination
	cycle_for_music_sensor             						// music_sensor
};

// idata *display_driver -> detailed display_driver
static code driver_init_pt const driver_init[11] =
{
	led_display_off_driver_init, 						// led_display_off
	moon_light_driver_init,						// moon_light
	fire_work_driver_init, 					// fire_work
	hot_wheels_driver_init,						// hot_wheels
	rainbow_chasing_driver_init, 						// rainbow_chasing
	color_chasing_7_driver_init,					// color_chasing_7
	color_breathe_3_driver_init, 						// color_breathe_3
	color_breathe_7_driver_init,						// color_breathe_7
	white_breathe_driver_init, 						// white_breathe
	combination_driver_init,						// combination
	music_sensor_driver_init 						// music_sensor
};


static code half_cycle_reached_pt const display_driver[11] = 
{
	led_display_off_driver, 						// led_display_off
	moon_light_driver,						// moon_light
	fire_work_driver, 					// fire_work
	hot_wheels_driver,						// hot_wheels
	rainbow_chasing_driver, 						// rainbow_chasing
	color_chasing_7_driver,					// color_chasing_7
	color_breathe_3_driver, 						// color_breathe_3
	color_breathe_7_driver,						// color_breathe_7
	white_breathe_driver, 						// white_breathe
	combination_driver,						// combination
	music_sensor_driver 						// music_sensor
};

static code display_driver_pt const half_cycle_callback[11] = 
{
	led_display_off_half_cycle_reached, 	// led_display_off
	moon_light_half_cycle_reached,			// moon_light
	fire_work_half_cycle_reached, 			// fire_work
	hot_wheels_half_cycle_reached,			// hot_wheels
	rainbow_chasing_half_cycle_reached, 	// rainbow_chasing
	color_chasing_7_half_cycle_reached,		// color_chasing_7
	color_breathe_3_cycle_reached, 			// color_breathe_3
	color_breathe_7_half_cycle_reached,		// color_breathe_7
	white_breathe_half_cycle_reached, 		// white_breathe
	combination_half_cycle_reached,			// combination
	music_sensor_half_cycle_reached		// music_sensor
};


void ctrl_speed_level_changed(const u8 speed) {
	if (check_speed_level(speed))
		led_logic_speed = speed;
}

void ctrl_lumins_changed(const u8 lumins) {
	u8 i;
	if (check_lumins_level(lumins))
	{
		for (i = 0; i < 19; ++i)
		{
			led_config_logic[i].lumins = lumins;
		}

	}
}


void ctrl_mode_changed(const u8 mode_code) {
	if (check_display_mode_type(mode_code) && led_mode_logic != (disp_t)mode_code) {
		led_mode_logic = (disp_t)mode_code;
		led_mode_set = led_mode_logic;
		led_mode_changed();
	}
	
} 
// ------------------------- public -------------------------
void display_startup_init() {
	led_flicker.cycle_output = OFF;
	led_flicker.cycle_enable = DISABLE;
	led_flicker.counter_for_cycle = 0;
	led_display_off_driver_init();
	led_config_change();
	// LED_OUT = 0;
}

void led_display_task() {
	led_cycle_based_adjust();
}


void led_mode_changed()  {
	driver_init_pt led_mode_driver_init = driver_init[led_mode_logic];
	
	if (led_mode_driver_init)
		led_mode_driver_init();
	led_flicker.counter_for_cycle = 0;
	led_config_change();
}

// ------------------------- private -------------------------

static void led_cycle_based_adjust() {
	if (led_flicker.cycle_enable) {
		if (led_flicker.counter_for_cycle >= cycle_params[led_mode_logic][led_logic_speed] >> 1)
			half_cycle_reached();

	} else {
		led_flicker.counter_for_cycle = 0;
	}
}


static void half_cycle_reached() {

	if (half_cycle_callback[led_mode_logic])
		half_cycle_callback[led_mode_logic]();
	led_config_change();
	led_flicker.counter_for_cycle = 0;
}


// ------------------------- Display Mode Code 0 --------------------------
static void led_display_off_driver(void) {

}

static void led_display_off_driver_init() {
	u8 i;
	led_flicker.cycle_output = OFF;
	led_flicker.cycle_enable = DISABLE;
	led_mode_logic = led_display_off;
	for(i = 0; i < 19; i++)
	{
		led_config_logic[i].red = 0;
		led_config_logic[i].green = 0;
		led_config_logic[i].blue = 0;
	}
}

static void led_display_off_half_cycle_reached()
{
	
}


// ------------------------- Display Mode Code 1 --------------------------
// 白光
static void moon_light_driver(void) {
	// led_config_logic.lumins = led_config_set.lumins;
		led_config_change();
}

static void moon_light_driver_init() {
	u8 i;
	led_flicker.cycle_output = ON;
	led_flicker.cycle_enable = DISABLE;
	
	// led_config_logic.lumins = led_config_set.lumins;
	led_mode_logic = moon_light;

	for(i = 0; i < 19; i++)
	{
		led_config_logic[i].red = 128;
		led_config_logic[i].green = 128;
		led_config_logic[i].blue = 128;
	}
}

static void moon_light_half_cycle_reached()
{
	
}

// ------------------------- Display Mode Code 2 --------------------------
// 烟花
// 左右两边同时按红/绿/翠绿/浅蓝/蓝/粉红/的顺序由下往上一个一个亮起
u8 cnt_for_cycle_fire_work = 0;
u8 cnt_for_one_cycle_fire_work = 0;
static void fire_work_driver(void) {
	// led_config_logic.lumins = led_config_set.lumins;
		led_config_change();
}

static void fire_work_driver_init() {
	u8 i;
	led_flicker.cycle_output = ON;
	led_flicker.cycle_enable = ENABLE;

	// led_config_logic.lumins = led_config_set.lumins;
	led_mode_logic = fire_work;
	cnt_for_cycle_fire_work = 0;
	cnt_for_one_cycle_fire_work = 0;
	for(i = 0; i < 19; i++)
	{
		led_config_logic[i].green = 0;
		led_config_logic[i].red = 0;
		led_config_logic[i].blue = 0;
	}
	// 红色
	led_config_logic[0].red = 255;
	led_config_logic[18].red = 255;
}

static void fire_work_half_cycle_reached() {
    u8 i;
	++ cnt_for_one_cycle_fire_work;
	if(cnt_for_one_cycle_fire_work == 10)
	{
		cnt_for_one_cycle_fire_work = 0;
		++ cnt_for_cycle_fire_work;
		if(cnt_for_cycle_fire_work == 6)
			cnt_for_cycle_fire_work = 0; 
	}
	if(!cnt_for_cycle_fire_work)
	{
		for(i = 0; i < 19; i++)
		{
			if(i == cnt_for_one_cycle_fire_work || i == 18 - cnt_for_one_cycle_fire_work)
				led_config_logic[i].red = 255;
			else
				led_config_logic[i].red = 0;
			led_config_logic[i].green = 0;
			led_config_logic[i].blue = 0;
		}
	} else if(cnt_for_cycle_fire_work == 1){
		for(i = 0; i < 19; i++)
		{
			if(i == cnt_for_one_cycle_fire_work || i == 18 - cnt_for_one_cycle_fire_work)
				led_config_logic[i].green = 255;
			else
				led_config_logic[i].green = 0;
			led_config_logic[i].red = 0;
			led_config_logic[i].blue = 0;
		}
	} else if(cnt_for_cycle_fire_work == 2){
		for(i = 0; i < 19; i++)
		{
			if(i == cnt_for_one_cycle_fire_work || i == 18 - cnt_for_one_cycle_fire_work)
			{
				led_config_logic[i].green = 201;
				led_config_logic[i].blue = 87;
				led_config_logic[i].red = 0;
			}
			else{
				led_config_logic[i].green = 0;
				led_config_logic[i].red = 0;
				led_config_logic[i].blue = 0;
			}
		}
	} else if(cnt_for_cycle_fire_work == 3){
		for(i = 0; i < 19; i++)
		{
			if(i == cnt_for_one_cycle_fire_work || i == 18 - cnt_for_one_cycle_fire_work)
			{
				led_config_logic[i].blue = 230;
				led_config_logic[i].green = 224;
				led_config_logic[i].red = 176;
			}
			else{
				led_config_logic[i].green = 0;
				led_config_logic[i].red = 0;
				led_config_logic[i].blue = 0;
			}
		}
	} else if(cnt_for_cycle_fire_work == 4){
		for(i = 0; i < 19; i++)
		{
			if(i == cnt_for_one_cycle_fire_work || i == 18 - cnt_for_one_cycle_fire_work)
			{
				led_config_logic[i].green = 0;
				led_config_logic[i].red = 0;
				led_config_logic[i].blue = 255;
			}
			else{
				led_config_logic[i].green = 0;
				led_config_logic[i].red = 0;
				led_config_logic[i].blue = 0;
			}
		}
	}
	else if(cnt_for_cycle_fire_work == 5){
		for(i = 0; i < 19; i++)
		{
			if(i == cnt_for_one_cycle_fire_work || i == 18 - cnt_for_one_cycle_fire_work)
			{
				led_config_logic[i].green = 192;
				led_config_logic[i].red = 255;
				led_config_logic[i].blue = 203;
			}
			else{
				led_config_logic[i].green = 0;
				led_config_logic[i].red = 0;
				led_config_logic[i].blue = 0;
			}
		}
	}
}

// ------------------------- Display Mode Code 3 --------------------------
//风火轮
//单个灯亮起、逆时针转一圈，换另外一个颜色逆时针转一圈，按照红/绿/翠绿/浅蓝/蓝/粉红/的顺序、依次循环
u8 cnt_for_cycle_hot_wheels = 0;
u8 cnt_for_one_cycle_hot_wheels = 0;

static void hot_wheels_driver() {
		led_config_change();
}

static void hot_wheels_driver_init() {
	u8 i;
	led_flicker.cycle_output = ON;
	led_flicker.cycle_enable = ENABLE;
	led_mode_logic = hot_wheels;
	cnt_for_cycle_hot_wheels = 0;
	cnt_for_one_cycle_hot_wheels = 0;
	for(i = 0; i < 19; i++)
	{
		if(i == cnt_for_one_cycle_hot_wheels)
			led_config_logic[i].red = 255;
		else
			led_config_logic[i].red = 0;
		led_config_logic[i].green = 0;
		led_config_logic[i].blue = 0;
	}
}

static void hot_wheels_half_cycle_reached() {
	u8 i;
	++ cnt_for_one_cycle_hot_wheels;
	if(cnt_for_one_cycle_hot_wheels == 19)
	{
		cnt_for_one_cycle_hot_wheels = 0;
		++ cnt_for_cycle_hot_wheels;
		if(cnt_for_cycle_hot_wheels == 6)
			cnt_for_cycle_hot_wheels = 0; 
	}
	// 红色/逆时针
	if(!cnt_for_cycle_hot_wheels)
	{
		for(i = 0; i < 19; i++)
		{
			if(i == cnt_for_one_cycle_hot_wheels)
				led_config_logic[i].red = 255;
			else
				led_config_logic[i].red = 0;
			led_config_logic[i].green = 0;
			led_config_logic[i].blue = 0;
		}
	// 绿色/顺时针
	} else if(cnt_for_cycle_hot_wheels == 1){
		for(i = 0; i < 19; i++)
		{
			if(i == cnt_for_one_cycle_hot_wheels)
				led_config_logic[i].green = 255;
			else
				led_config_logic[i].green = 0;
			led_config_logic[i].red = 0;
			led_config_logic[i].blue = 0;
		}
	// 翠绿/逆时针
	}  else if(cnt_for_cycle_hot_wheels == 2){
		for(i = 0; i < 19; i++)
		{
			if(i == cnt_for_one_cycle_hot_wheels)
			{
				led_config_logic[i].green = 201;
				led_config_logic[i].blue = 87;
				led_config_logic[i].red = 0;
			}
			else
			{
				led_config_logic[i].green = 0;
				led_config_logic[i].blue = 0;
				led_config_logic[i].red = 0;
			}
		}
	// 浅蓝/顺时针
	} else if(cnt_for_cycle_hot_wheels == 3){
		for(i = 0; i < 19; i++)
		{
			if(i == cnt_for_one_cycle_hot_wheels)
			{
				led_config_logic[i].blue = 230;
				led_config_logic[i].green = 224;
				led_config_logic[i].red = 176;
			}
			else
			{
				led_config_logic[i].blue = 0;
				led_config_logic[i].green = 0;
				led_config_logic[i].red = 0;
			}
			
			
		}
	}
	// 蓝/逆时针
 	else if(cnt_for_cycle_hot_wheels == 4){
		for(i = 0; i < 19; i++)
		{
			if(i == cnt_for_one_cycle_hot_wheels)
				led_config_logic[i].blue = 255;
			else
				led_config_logic[i].blue = 0;
			led_config_logic[i].green = 0;
			led_config_logic[i].red = 0;
		}
	}
	// 粉红/顺时针
	 else if(cnt_for_cycle_hot_wheels == 5){
		for(i = 0; i < 19; i++)
		{
			if(i == cnt_for_one_cycle_hot_wheels)
			{
				led_config_logic[i].green = 192;
				led_config_logic[i].red = 255;
				led_config_logic[i].blue = 203;
			}
			else
			{
				led_config_logic[i].green = 0;
				led_config_logic[i].red = 0;
				led_config_logic[i].blue = 0;
			}

		}
	}
}

// ------------------------- Display Mode Code 4 --------------------------
// 彩虹追逐
// 红/绿/翠绿/浅蓝/蓝/粉红/六种颜色同时亮起形成一个半圆，左右摆动。
// todo
u8 cnt_for_cycle_rainbow_chasing = 0;

static void rainbow_chasing_driver(void) {
		led_config_change();
}

static void rainbow_chasing_driver_init() {
	u8 i;
	led_flicker.cycle_output = ON;
	led_flicker.cycle_enable = ENABLE;
	led_mode_logic = rainbow_chasing;
	cnt_for_cycle_rainbow_chasing = 0;
	for(i = 0; i < 19; i++)
	{
		led_config_logic[i].green = 0;
		led_config_logic[i].red = 0;
		led_config_logic[i].blue = 0;
	}
		// 红色
		led_config_logic[0].red = 255;
		// 绿色
		led_config_logic[1].green = 255;
		// 翠绿
		led_config_logic[2].green = 201;
		led_config_logic[2].blue = 87;
		// 浅蓝
		led_config_logic[3].blue = 230;
		led_config_logic[3].green = 224;
		led_config_logic[3].red = 176;
		// 蓝
		led_config_logic[4].blue = 255;
		// 粉红
		led_config_logic[5].green = 192;
		led_config_logic[5].red = 255;
		led_config_logic[5].blue = 203;
}

static void rainbow_chasing_half_cycle_reached() {
		u8 i;
	for(i = 0; i < 19; i++)
	{
		led_config_logic[i].green = 0;
		led_config_logic[i].red = 0;
		led_config_logic[i].blue = 0;
	}
	++ cnt_for_cycle_rainbow_chasing;
	if(cnt_for_cycle_rainbow_chasing == 26)
		cnt_for_cycle_rainbow_chasing = 0;
	if(!cnt_for_cycle_rainbow_chasing)
	{
		// 红色
		led_config_logic[0].red = 255;
		// 绿色
		led_config_logic[1].green = 255;
		// 翠绿
		led_config_logic[2].green = 201;
		led_config_logic[2].blue = 87;
		// 浅蓝
		led_config_logic[3].blue = 230;
		led_config_logic[3].green = 224;
		led_config_logic[3].red = 176;
		// 蓝
		led_config_logic[4].blue = 255;
		// 粉红
		led_config_logic[5].green = 192;
		led_config_logic[5].red = 255;
		led_config_logic[5].blue = 203;
	} else if(cnt_for_cycle_rainbow_chasing == 1 || cnt_for_cycle_rainbow_chasing == 25){
		// 红色
		led_config_logic[1].red = 255;
		// 绿色
		led_config_logic[2].green = 255;
		// 翠绿
		led_config_logic[3].green = 201;
		led_config_logic[3].blue = 87;
		// 浅蓝
		led_config_logic[4].blue = 230;
		led_config_logic[4].green = 224;
		led_config_logic[4].red = 176;
		// 蓝
		led_config_logic[5].blue = 255;
		// 粉红
		led_config_logic[6].green = 192;
		led_config_logic[6].red = 255;
		led_config_logic[6].blue = 203;
	} else if(cnt_for_cycle_rainbow_chasing == 2  || cnt_for_cycle_rainbow_chasing == 24){
		// 红色
		led_config_logic[2].red = 255;
		// 绿色
		led_config_logic[3].green = 255;
		// 翠绿
		led_config_logic[4].green = 201;
		led_config_logic[4].blue = 87;
		// 浅蓝
		led_config_logic[5].blue = 230;
		led_config_logic[5].green = 224;
		led_config_logic[5].red = 176;
		// 蓝
		led_config_logic[6].blue = 255;
		// 粉红
		led_config_logic[7].green = 192;
		led_config_logic[7].red = 255;
		led_config_logic[7].blue = 203;
	} else if(cnt_for_cycle_rainbow_chasing == 3  || cnt_for_cycle_rainbow_chasing == 23){
		// 红色
		led_config_logic[3].red = 255;
		// 绿色
		led_config_logic[4].green = 255;
		// 翠绿
		led_config_logic[5].green = 201;
		led_config_logic[5].blue = 87;
		// 浅蓝
		led_config_logic[6].blue = 230;
		led_config_logic[6].green = 224;
		led_config_logic[6].red = 176;
		// 蓝
		led_config_logic[7].blue = 255;
		// 粉红
		led_config_logic[8].green = 192;
		led_config_logic[8].red = 255;
		led_config_logic[8].blue = 203;
	} else if(cnt_for_cycle_rainbow_chasing == 4 || cnt_for_cycle_rainbow_chasing == 22){
		// 红色
		led_config_logic[4].red = 255;
		// 绿色
		led_config_logic[5].green = 255;
		// 翠绿
		led_config_logic[6].green = 201;
		led_config_logic[6].blue = 87;
		// 浅蓝
		led_config_logic[7].blue = 230;
		led_config_logic[7].green = 224;
		led_config_logic[7].red = 176;
		// 蓝
		led_config_logic[8].blue = 255;
		// 粉红
		led_config_logic[9].green = 192;
		led_config_logic[9].red = 255;
		led_config_logic[9].blue = 203;
	} else if(cnt_for_cycle_rainbow_chasing == 5  || cnt_for_cycle_rainbow_chasing == 21){
				// 红色
		led_config_logic[5].red = 255;
		// 绿色
		led_config_logic[6].green = 255;
		// 翠绿
		led_config_logic[7].green = 201;
		led_config_logic[7].blue = 87;
		// 浅蓝
		led_config_logic[8].blue = 230;
		led_config_logic[8].green = 224;
		led_config_logic[8].red = 176;
		// 蓝
		led_config_logic[9].blue = 255;
		// 粉红
		led_config_logic[10].green = 192;
		led_config_logic[10].red = 255;
		led_config_logic[10].blue = 203;
	} else if(cnt_for_cycle_rainbow_chasing == 6 || cnt_for_cycle_rainbow_chasing == 20){
				// 红色
		led_config_logic[6].red = 255;
		// 绿色
		led_config_logic[7].green = 255;
		// 翠绿
		led_config_logic[8].green = 201;
		led_config_logic[8].blue = 87;
		// 浅蓝
		led_config_logic[9].blue = 230;
		led_config_logic[9].green = 224;
		led_config_logic[9].red = 176;
		// 蓝
		led_config_logic[10].blue = 255;
		// 粉红
		led_config_logic[11].green = 192;
		led_config_logic[11].red = 255;
		led_config_logic[11].blue = 203;
	} else if(cnt_for_cycle_rainbow_chasing == 7 || cnt_for_cycle_rainbow_chasing == 19){
				// 红色
		led_config_logic[7].red = 255;
		// 绿色
		led_config_logic[8].green = 255;
		// 翠绿
		led_config_logic[9].green = 201;
		led_config_logic[9].blue = 87;
		// 浅蓝
		led_config_logic[10].blue = 230;
		led_config_logic[10].green = 224;
		led_config_logic[10].red = 176;
		// 蓝
		led_config_logic[11].blue = 255;
		// 粉红
		led_config_logic[12].green = 192;
		led_config_logic[12].red = 255;
		led_config_logic[12].blue = 203;
	} else if(cnt_for_cycle_rainbow_chasing == 8 || cnt_for_cycle_rainbow_chasing == 18){
				// 红色
		led_config_logic[8].red = 255;
		// 绿色
		led_config_logic[9].green = 255;
		// 翠绿
		led_config_logic[10].green = 201;
		led_config_logic[10].blue = 87;
		// 浅蓝
		led_config_logic[11].blue = 230;
		led_config_logic[11].green = 224;
		led_config_logic[11].red = 176;
		// 蓝
		led_config_logic[12].blue = 255;
		// 粉红
		led_config_logic[13].green = 192;
		led_config_logic[13].red = 255;
		led_config_logic[13].blue = 203;
	} else if(cnt_for_cycle_rainbow_chasing == 9 || cnt_for_cycle_rainbow_chasing == 17){
				// 红色
		led_config_logic[9].red = 255;
		// 绿色
		led_config_logic[10].green = 255;
		// 翠绿
		led_config_logic[11].green = 201;
		led_config_logic[11].blue = 87;
		// 浅蓝
		led_config_logic[12].blue = 230;
		led_config_logic[12].green = 224;
		led_config_logic[12].red = 176;
		// 蓝
		led_config_logic[13].blue = 255;
		// 粉红
		led_config_logic[14].green = 192;
		led_config_logic[14].red = 255;
		led_config_logic[14].blue = 203;
	} else if(cnt_for_cycle_rainbow_chasing == 10 || cnt_for_cycle_rainbow_chasing == 16){
				// 红色
		led_config_logic[10].red = 255;
		// 绿色
		led_config_logic[11].green = 255;
		// 翠绿
		led_config_logic[12].green = 201;
		led_config_logic[12].blue = 87;
		// 浅蓝
		led_config_logic[13].blue = 230;
		led_config_logic[13].green = 224;
		led_config_logic[13].red = 176;
		// 蓝
		led_config_logic[14].blue = 255;
		// 粉红
		led_config_logic[15].green = 192;
		led_config_logic[15].red = 255;
		led_config_logic[15].blue = 203;
	} else if(cnt_for_cycle_rainbow_chasing == 11 || cnt_for_cycle_rainbow_chasing == 15){
				// 红色
		led_config_logic[11].red = 255;
		// 绿色
		led_config_logic[12].green = 255;
		// 翠绿
		led_config_logic[13].green = 201;
		led_config_logic[13].blue = 87;
		// 浅蓝
		led_config_logic[14].blue = 230;
		led_config_logic[14].green = 224;
		led_config_logic[14].red = 176;
		// 蓝
		led_config_logic[15].blue = 255;
		// 粉红
		led_config_logic[16].green = 192;
		led_config_logic[16].red = 255;
		led_config_logic[16].blue = 203;
	} else if(cnt_for_cycle_rainbow_chasing == 12 || cnt_for_cycle_rainbow_chasing == 14){
				// 红色
		led_config_logic[12].red = 255;
		// 绿色
		led_config_logic[13].green = 255;
		// 翠绿
		led_config_logic[14].green = 201;
		led_config_logic[14].blue = 87;
		// 浅蓝
		led_config_logic[15].blue = 230;
		led_config_logic[15].green = 224;
		led_config_logic[15].red = 176;
		// 蓝
		led_config_logic[16].blue = 255;
		// 粉红
		led_config_logic[17].green = 192;
		led_config_logic[17].red = 255;
		led_config_logic[17].blue = 203;
	} else if(cnt_for_cycle_rainbow_chasing == 13){
				// 红色
		led_config_logic[13].red = 255;
		// 绿色
		led_config_logic[14].green = 255;
		// 翠绿
		led_config_logic[15].green = 201;
		led_config_logic[15].blue = 87;
		// 浅蓝
		led_config_logic[16].blue = 230;
		led_config_logic[16].green = 224;
		led_config_logic[16].red = 176;
		// 蓝
		led_config_logic[17].blue = 255;
		// 粉红
		led_config_logic[18].green = 192;
		led_config_logic[18].red = 255;
		led_config_logic[18].blue = 203;
	}
}

// ------------------------- Display Mode Code 5 --------------------------
// 七色追逐
// 七种灯光间隔一个旋转变化
// 白/红/绿/翠绿/浅蓝/蓝/粉红
// todo
u8 cnt_for_cycle_color_chasing_7 = 0;
static void color_chasing_7_driver(void) {
		led_config_change();
}

static void color_chasing_7_driver_init() {
	u8 i;
	for(i = 0; i < 19; i++)
	{
		led_config_logic[i].red = 0;
		led_config_logic[i].green = 0;
		led_config_logic[i].blue = 0;
	}
	led_flicker.cycle_output = ON;
	led_flicker.cycle_enable = ENABLE;

	// led_config_logic.lumins = led_config_set.lumins;
	led_mode_logic = color_chasing_7;
	cnt_for_cycle_color_chasing_7 = 0;
	// 白/红/绿/翠绿/浅蓝/蓝/粉红/白/红/绿/翠绿/浅蓝/蓝/粉红/白/红/绿/翠绿
		led_config_logic[0].green = 255;
		led_config_logic[0].red = 255;
		led_config_logic[0].blue = 255;

		led_config_logic[1].red = 255;

		led_config_logic[2].green = 255;

		led_config_logic[3].green = 201;
		led_config_logic[3].blue = 87;

		led_config_logic[4].blue = 230;
		led_config_logic[4].green = 224;
		led_config_logic[4].red = 176;

		led_config_logic[5].green = 255;

		led_config_logic[6].green = 192;
		led_config_logic[6].red = 255;
		led_config_logic[6].blue = 203;

		led_config_logic[7].green = 255;
		led_config_logic[7].red = 255;
		led_config_logic[7].blue = 255;

		led_config_logic[8].red = 255;

		led_config_logic[9].green = 255;

		led_config_logic[10].green = 201;
		led_config_logic[10].blue = 87;

		led_config_logic[11].blue = 230;
		led_config_logic[11].green = 224;
		led_config_logic[11].red = 176;

		led_config_logic[12].green = 255;

		led_config_logic[13].green = 192;
		led_config_logic[13].red = 255;
		led_config_logic[13].blue = 203;

		led_config_logic[14].green = 255;
		led_config_logic[14].red = 255;
		led_config_logic[14].blue = 255;

		led_config_logic[15].red = 255;

		led_config_logic[16].green = 255;

		led_config_logic[17].green = 201;
		led_config_logic[17].blue = 87;

		led_config_logic[18].blue = 230;
		led_config_logic[18].green = 224;
		led_config_logic[18].red = 176;


}

static void color_chasing_7_half_cycle_reached() {
		u8 i;
	++ cnt_for_cycle_color_chasing_7;
	if(cnt_for_cycle_color_chasing_7 == 7)
		cnt_for_cycle_color_chasing_7 = 0;
	for(i = 0; i < 19; i++)
	{
		led_config_logic[i].green = 0;
		led_config_logic[i].red = 0;
		led_config_logic[i].blue = 0;
	}

	if(!cnt_for_cycle_color_chasing_7){
		// 白/红/绿/翠绿/浅蓝/蓝/粉红/白/红/绿/翠绿/浅蓝/蓝/粉红
		led_config_logic[0].green = 255;
		led_config_logic[0].red = 255;
		led_config_logic[0].blue = 255;

		led_config_logic[1].red = 255;

		led_config_logic[2].green = 255;

		led_config_logic[3].green = 201;
		led_config_logic[3].blue = 87;

		led_config_logic[4].blue = 230;
		led_config_logic[4].green = 224;
		led_config_logic[4].red = 176;

		led_config_logic[5].green = 255;

		led_config_logic[6].green = 192;
		led_config_logic[6].red = 255;
		led_config_logic[6].blue = 203;

		led_config_logic[7].green = 255;
		led_config_logic[7].red = 255;
		led_config_logic[7].blue = 255;

		led_config_logic[8].red = 255;

		led_config_logic[9].green = 255;

		led_config_logic[10].green = 201;
		led_config_logic[10].blue = 87;

		led_config_logic[11].blue = 230;
		led_config_logic[11].green = 224;
		led_config_logic[11].red = 176;

		led_config_logic[12].green = 255;

		led_config_logic[13].green = 192;
		led_config_logic[13].red = 255;
		led_config_logic[13].blue = 203;

		led_config_logic[14].green = 255;
		led_config_logic[14].red = 255;
		led_config_logic[14].blue = 255;

		led_config_logic[15].red = 255;

		led_config_logic[16].green = 255;

		led_config_logic[17].green = 201;
		led_config_logic[17].blue = 87;

		led_config_logic[18].blue = 230;
		led_config_logic[18].green = 224;
		led_config_logic[18].red = 176;


	} else if(cnt_for_cycle_color_chasing_7 == 1) {
		// 粉红/白/红/绿/翠绿/浅蓝/蓝/粉红/白/红/绿/翠绿/浅蓝/蓝/粉红/白/红/绿
		led_config_logic[0].green = 192;
		led_config_logic[0].red = 255;
		led_config_logic[0].blue = 203;

		led_config_logic[1].green = 255;
		led_config_logic[1].red = 255;
		led_config_logic[1].blue = 255;

		led_config_logic[2].red = 255;

		led_config_logic[3].green = 255;

		led_config_logic[4].green = 201;
		led_config_logic[4].blue = 87;

		led_config_logic[5].blue = 230;
		led_config_logic[5].green = 224;
		led_config_logic[5].red = 176;

		led_config_logic[6].green = 255;

		led_config_logic[7].green = 192;
		led_config_logic[7].red = 255;
		led_config_logic[7].blue = 203;

		led_config_logic[8].green = 255;
		led_config_logic[8].red = 255;
		led_config_logic[8].blue = 255;

		led_config_logic[9].red = 255;

		led_config_logic[10].green = 255;

		led_config_logic[11].green = 201;
		led_config_logic[11].blue = 87;

		led_config_logic[12].blue = 230;
		led_config_logic[12].green = 224;
		led_config_logic[12].red = 176;

		led_config_logic[13].green = 255;

		led_config_logic[14].green = 192;
		led_config_logic[14].red = 255;
		led_config_logic[14].blue = 203;

		led_config_logic[15].green = 255;
		led_config_logic[15].red = 255;
		led_config_logic[15].blue = 255;

		led_config_logic[16].red = 255;

		led_config_logic[17].green = 255;

		led_config_logic[18].green = 201;
		led_config_logic[18].blue = 87;

	} else if(cnt_for_cycle_color_chasing_7 == 2) {
		// 蓝/粉红/白/红/绿/翠绿/浅蓝/蓝/粉红/白/红/绿/翠绿/浅蓝/蓝/粉红/白/红

		led_config_logic[0].green = 255;

		led_config_logic[1].green = 192;
		led_config_logic[1].red = 255;
		led_config_logic[1].blue = 203;

		led_config_logic[2].green = 255;
		led_config_logic[2].red = 255;
		led_config_logic[2].blue = 255;

		led_config_logic[3].red = 255;

		led_config_logic[4].green = 255;

		led_config_logic[5].green = 201;
		led_config_logic[5].blue = 87;

		led_config_logic[6].blue = 230;
		led_config_logic[6].green = 224;
		led_config_logic[6].red = 176;

		led_config_logic[7].green = 255;

		led_config_logic[8].green = 192;
		led_config_logic[8].red = 255;
		led_config_logic[8].blue = 203;

		led_config_logic[9].green = 255;
		led_config_logic[9].red = 255;
		led_config_logic[9].blue = 255;

		led_config_logic[10].red = 255;

		led_config_logic[11].green = 255;

		led_config_logic[12].green = 201;
		led_config_logic[12].blue = 87;

		led_config_logic[13].blue = 230;
		led_config_logic[13].green = 224;
		led_config_logic[13].red = 176;

		led_config_logic[14].green = 255;

		led_config_logic[15].green = 192;
		led_config_logic[15].red = 255;
		led_config_logic[15].blue = 203;

		led_config_logic[16].green = 255;
		led_config_logic[16].red = 255;
		led_config_logic[16].blue = 255;

		led_config_logic[17].red = 255;

		led_config_logic[18].green = 255;


	} else if(cnt_for_cycle_color_chasing_7 == 3) {
		// 浅蓝/蓝/粉红/白/红/绿/翠绿/浅蓝/蓝/粉红/白/红/绿/翠绿/浅蓝/蓝/粉红/白
		led_config_logic[0].blue = 230;
		led_config_logic[0].green = 224;
		led_config_logic[0].red = 176;


		led_config_logic[1].green = 255;

		led_config_logic[2].green = 192;
		led_config_logic[2].red = 255;
		led_config_logic[2].blue = 203;

		led_config_logic[3].green = 255;
		led_config_logic[3].red = 255;
		led_config_logic[3].blue = 255;

		led_config_logic[4].red = 255;

		led_config_logic[5].green = 255;

		led_config_logic[6].green = 201;
		led_config_logic[6].blue = 87;

		led_config_logic[7].blue = 230;
		led_config_logic[7].green = 224;
		led_config_logic[7].red = 176;

		led_config_logic[8].green = 255;

		led_config_logic[9].green = 192;
		led_config_logic[9].red = 255;
		led_config_logic[9].blue = 203;

		led_config_logic[10].green = 255;
		led_config_logic[10].red = 255;
		led_config_logic[10].blue = 255;

		led_config_logic[11].red = 255;

		led_config_logic[12].green = 255;

		led_config_logic[13].green = 201;
		led_config_logic[13].blue = 87;

		led_config_logic[14].blue = 230;
		led_config_logic[14].green = 224;
		led_config_logic[14].red = 176;

		led_config_logic[15].green = 255;

		led_config_logic[16].green = 192;
		led_config_logic[16].red = 255;
		led_config_logic[16].blue = 203;

		led_config_logic[17].green = 255;
		led_config_logic[17].red = 255;
		led_config_logic[17].blue = 255;

		led_config_logic[18].red = 255;

	} else if(cnt_for_cycle_color_chasing_7 == 4) {
		// 翠绿/浅蓝/蓝/粉红/白/红/绿/翠绿/浅蓝/蓝/粉红/白/红/绿/翠绿/浅蓝/蓝/粉红
		led_config_logic[0].green = 201;
		led_config_logic[0].blue = 87;

		led_config_logic[1].blue = 230;
		led_config_logic[1].green = 224;
		led_config_logic[1].red = 176;


		led_config_logic[2].green = 255;

		led_config_logic[3].green = 192;
		led_config_logic[3].red = 255;
		led_config_logic[3].blue = 203;

		led_config_logic[4].green = 255;
		led_config_logic[4].red = 255;
		led_config_logic[4].blue = 255;

		led_config_logic[5].red = 255;

		led_config_logic[6].green = 255;

		led_config_logic[7].green = 201;
		led_config_logic[7].blue = 87;

		led_config_logic[8].blue = 230;
		led_config_logic[8].green = 224;
		led_config_logic[8].red = 176;

		led_config_logic[9].green = 255;

		led_config_logic[10].green = 192;
		led_config_logic[10].red = 255;
		led_config_logic[10].blue = 203;

		led_config_logic[11].green = 255;
		led_config_logic[11].red = 255;
		led_config_logic[11].blue = 255;

		led_config_logic[12].red = 255;

		led_config_logic[13].green = 255;

		led_config_logic[14].green = 201;
		led_config_logic[14].blue = 87;

		led_config_logic[15].blue = 230;
		led_config_logic[15].green = 224;
		led_config_logic[15].red = 176;

		led_config_logic[16].green = 255;

		led_config_logic[17].green = 192;
		led_config_logic[17].red = 255;
		led_config_logic[17].blue = 203;

		led_config_logic[18].green = 255;
		led_config_logic[18].red = 255;
		led_config_logic[18].blue = 255;


	} else if(cnt_for_cycle_color_chasing_7 == 5) {
		// 绿/翠绿/浅蓝/蓝/粉红/白/红/绿/翠绿/浅蓝/蓝/粉红/白/红/绿/翠绿/浅蓝/蓝
		led_config_logic[0].green = 255;

		led_config_logic[1].green = 201;
		led_config_logic[1].blue = 87;

		led_config_logic[2].blue = 230;
		led_config_logic[2].green = 224;
		led_config_logic[2].red = 176;


		led_config_logic[3].green = 255;

		led_config_logic[4].green = 192;
		led_config_logic[4].red = 255;
		led_config_logic[4].blue = 203;

		led_config_logic[5].green = 255;
		led_config_logic[5].red = 255;
		led_config_logic[5].blue = 255;

		led_config_logic[6].red = 255;

		led_config_logic[7].green = 255;

		led_config_logic[8].green = 201;
		led_config_logic[8].blue = 87;

		led_config_logic[9].blue = 230;
		led_config_logic[9].green = 224;
		led_config_logic[9].red = 176;

		led_config_logic[10].green = 255;

		led_config_logic[11].green = 192;
		led_config_logic[11].red = 255;
		led_config_logic[11].blue = 203;

		led_config_logic[12].green = 255;
		led_config_logic[12].red = 255;
		led_config_logic[12].blue = 255;

		led_config_logic[13].red = 255;

		led_config_logic[14].green = 255;

		led_config_logic[15].green = 201;
		led_config_logic[15].blue = 87;

		led_config_logic[16].blue = 230;
		led_config_logic[16].green = 224;
		led_config_logic[16].red = 176;

		led_config_logic[17].green = 255;

		led_config_logic[18].green = 192;
		led_config_logic[18].red = 255;
		led_config_logic[18].blue = 203;



	} else if(cnt_for_cycle_color_chasing_7 == 6) {
		// 红/绿/翠绿/浅蓝/蓝/粉红/白/红/绿/翠绿/浅蓝/蓝/粉红/白/红/绿/翠绿/浅蓝
		led_config_logic[0].red = 255;

		led_config_logic[1].green = 255;

		led_config_logic[2].green = 201;
		led_config_logic[2].blue = 87;

		led_config_logic[3].blue = 230;
		led_config_logic[3].green = 224;
		led_config_logic[3].red = 176;


		led_config_logic[4].green = 255;

		led_config_logic[5].green = 192;
		led_config_logic[5].red = 255;
		led_config_logic[5].blue = 203;

		led_config_logic[6].green = 255;
		led_config_logic[6].red = 255;
		led_config_logic[6].blue = 255;

		led_config_logic[7].red = 255;

		led_config_logic[8].green = 255;

		led_config_logic[9].green = 201;
		led_config_logic[9].blue = 87;

		led_config_logic[10].blue = 230;
		led_config_logic[10].green = 224;
		led_config_logic[10].red = 176;

		led_config_logic[11].green = 255;

        led_config_logic[12].green = 192;
		led_config_logic[12].red = 255;
		led_config_logic[12].blue = 203;

		led_config_logic[13].green = 255;
		led_config_logic[13].red = 255;
		led_config_logic[13].blue = 255;

		led_config_logic[14].red = 255;

		led_config_logic[15].green = 255;

		led_config_logic[16].green = 201;
		led_config_logic[16].blue = 87;

		led_config_logic[17].blue = 230;
		led_config_logic[17].green = 224;
		led_config_logic[17].red = 176;

		led_config_logic[18].green = 255;
	} 
}

// ------------------------- Display Mode Code 6 --------------------------
u16 cnt_for_cycle_color_breathe_3 = 0; 
static void color_breathe_3_driver(void) {
		led_config_change();
}

static void color_breathe_3_driver_init() {
	u8 i;
	for(i = 0; i < 19; i++)
	{
		led_config_logic[i].red = 1;
		led_config_logic[i].green = 0;
		led_config_logic[i].blue = 0;
	}
	led_flicker.cycle_output = ON;
	led_flicker.cycle_enable = ENABLE;

	// led_config_logic.lumins = led_config_set.lumins;
	led_mode_logic = color_breathe_3;
	cnt_for_cycle_color_breathe_3 = 0;
}

static void color_breathe_3_cycle_reached() {
	// 红色
	u8 i;
	++ cnt_for_cycle_color_breathe_3;
	if(cnt_for_cycle_color_breathe_3 >= 1500)
		cnt_for_cycle_color_breathe_3 = 0;
	if(cnt_for_cycle_color_breathe_3 < 250)
	{
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].red++;
			led_config_logic[i].green = 0;
			led_config_logic[i].blue = 0;
		}

	} else if(cnt_for_cycle_color_breathe_3 < 500 && cnt_for_cycle_color_breathe_3 >= 250){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].red --;
			led_config_logic[i].green = 0;
			led_config_logic[i].blue = 0;
		}
	}
	else if(cnt_for_cycle_color_breathe_3 < 750 && cnt_for_cycle_color_breathe_3 >= 500){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].red = 0;
			led_config_logic[i].green ++;
			led_config_logic[i].blue = 0;
		}
	}

	else if(cnt_for_cycle_color_breathe_3 < 1000 && cnt_for_cycle_color_breathe_3 >= 750){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].red = 0;
			led_config_logic[i].green --;
			led_config_logic[i].blue = 0;
		}
	}

	// 蓝色
	 else if(cnt_for_cycle_color_breathe_3 < 1250 && cnt_for_cycle_color_breathe_3 >= 1000){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].red = 0;
			led_config_logic[i].green = 0;
			led_config_logic[i].blue ++;
		}
	}  else if(cnt_for_cycle_color_breathe_3 < 1500 && cnt_for_cycle_color_breathe_3 >= 1250){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].red = 0;
			led_config_logic[i].green = 0;
			led_config_logic[i].blue --;
		}
	}
}

// ------------------------- Display Mode Code 7 --------------------------
u16 cnt_for_cycle_breathe_7 = 0;
static void color_breathe_7_driver(void) {
		led_config_change();
}

static void color_breathe_7_driver_init() {
	u8 i;
	for(i = 0; i < 19; i++)
	{
		led_config_logic[i].red = 1;
		led_config_logic[i].green = 1;
		led_config_logic[i].blue = 1;
	}
	led_flicker.cycle_output = ON;
	led_flicker.cycle_enable = ENABLE;

	// led_config_logic.lumins = led_config_set.lumins;
	led_mode_logic = color_breathe_7;
	cnt_for_cycle_breathe_7 = 0;
}

static void color_breathe_7_half_cycle_reached() {
    u8 i;
	++ cnt_for_cycle_breathe_7;
	if(cnt_for_cycle_breathe_7 >= 1760)
	{
		cnt_for_cycle_breathe_7 = 0;
		for(i = 0; i < 19; i++)
		{
		  led_config_logic[i].red = 0;
		  led_config_logic[i].green = 0;
		  led_config_logic[i].blue = 0;
		}
	}
	// 白色
	if(cnt_for_cycle_breathe_7 < 250)
	{
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].red++;
			led_config_logic[i].green++;
			led_config_logic[i].blue++;
		}

	} else if(cnt_for_cycle_breathe_7 < 500 && cnt_for_cycle_breathe_7 >= 250){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].red --;
			led_config_logic[i].green --;
			led_config_logic[i].blue --;
		}
	}
	// 红色 
	else if(cnt_for_cycle_breathe_7 < 750 && cnt_for_cycle_breathe_7 >= 500) {
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].red++;
			led_config_logic[i].green = 0;
			led_config_logic[i].blue = 0;
		}

	} else if(cnt_for_cycle_breathe_7 < 1000 && cnt_for_cycle_breathe_7 >= 750){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].red --;
			led_config_logic[i].green = 0;
			led_config_logic[i].blue = 0;
		}
	} 
	// 绿色
	else if(cnt_for_cycle_breathe_7 < 1250 && cnt_for_cycle_breathe_7 >= 1000){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].red = 0;
			led_config_logic[i].green ++;
			led_config_logic[i].blue = 0;
		}
	} else if(cnt_for_cycle_breathe_7 < 1500 && cnt_for_cycle_breathe_7 >= 1250){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].red = 0;
			led_config_logic[i].green --;
			led_config_logic[i].blue = 0;
		}
	}
	// 蓝色
	 else if(cnt_for_cycle_breathe_7 < 1250 && cnt_for_cycle_breathe_7 >= 1000){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].red = 0;
			led_config_logic[i].green = 0;
			led_config_logic[i].blue ++;
		}
	}  else if(cnt_for_cycle_breathe_7 < 1500 && cnt_for_cycle_breathe_7 >= 1250){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].red = 0;
			led_config_logic[i].green = 0;
			led_config_logic[i].blue --;
		}
	} 
	// 浅蓝色
	 else if(cnt_for_cycle_breathe_7 < 1520 && cnt_for_cycle_breathe_7 >= 1500){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].blue = led_config_logic[i].blue + 10;
			led_config_logic[i].green = led_config_logic[i].green + 9;
			led_config_logic[i].red = led_config_logic[i].red + 6;
		}
	}  else if(cnt_for_cycle_breathe_7 < 1540 && cnt_for_cycle_breathe_7 >= 1520){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].blue -= 10;
			led_config_logic[i].green -= 9;
			led_config_logic[i].red -= 6;
		}
	} 

// 翠绿色
	 else if(cnt_for_cycle_breathe_7 < 1610 && cnt_for_cycle_breathe_7 >= 1540){
		for(i = 0; i < 19; i++)
		{
          led_config_logic[i].green += 3;
		  led_config_logic[i].blue += 1;
		  led_config_logic[i].red = 0;
		}
	}  else if(cnt_for_cycle_breathe_7 < 1680 && cnt_for_cycle_breathe_7 >= 1610){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].green -= 3;
			led_config_logic[i].blue -= 1;
			led_config_logic[i].red = 0;
		}
	} 
// 粉红
	else if(cnt_for_cycle_breathe_7 < 1720 && cnt_for_cycle_breathe_7 >= 1680){
		for(i = 0; i < 19; i++)
		{
           led_config_logic[i].green += 4;
		   led_config_logic[i].red += 5;
		   led_config_logic[i].blue += 4;
		}
	}  else if(cnt_for_cycle_breathe_7 < 1760 && cnt_for_cycle_breathe_7 >= 1720){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].green -= 4;
			led_config_logic[i].red -= 5;
			led_config_logic[i].blue -= 4;
		}
	} 
}

// ------------------------- Display Mode Code 8 --------------------------
// 白呼吸
// 白慢慢呼吸变化
u16 cnt_for_cycle_white_breathe = 0;
static void white_breathe_driver(void) {
		led_config_change();
}

static void white_breathe_driver_init() {
	u8 i;
	for(i = 0; i < 19; i++)
	{
		led_config_logic[i].red = 1;
		led_config_logic[i].green = 1;
		led_config_logic[i].blue = 1;
	}
	led_flicker.cycle_output = ON;
	led_flicker.cycle_enable = ENABLE;
	cnt_for_cycle_white_breathe = 0;

	// led_config_logic.lumins = led_config_set.lumins;
	led_mode_logic = white_breathe;
}

static void white_breathe_half_cycle_reached() {
    u8 i;
	++ cnt_for_cycle_white_breathe;
	if(cnt_for_cycle_white_breathe >= 500)
		cnt_for_cycle_white_breathe = 0;
	if(cnt_for_cycle_white_breathe < 250)
	{
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].red++;
			led_config_logic[i].green++;
			led_config_logic[i].blue++;
		}

	} else if(cnt_for_cycle_white_breathe < 500 && cnt_for_cycle_white_breathe >= 250){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].red --;
			led_config_logic[i].green --;
			led_config_logic[i].blue --;
		}
	}
}
// ------------------------- Display Mode Code 9 --------------------------
// 七种颜色慢慢自动切换

u8 cnt_for_cycle_combination = 0;

static void combination_driver(void) {
		led_config_change();
}

static void combination_driver_init() {
	u8 i;
	for(i = 0; i < 19; i++)
	{
		led_config_logic[i].red = 255;
		led_config_logic[i].green = 255;
		led_config_logic[i].blue = 255;
	}
	led_flicker.cycle_output = ON;
	led_flicker.cycle_enable = ENABLE;

	// led_config_logic.lumins = led_config_set.lumins;
	led_mode_logic = combination;

	cnt_for_cycle_combination = 0;
}

static void combination_half_cycle_reached() {
	u8 i;
	++ cnt_for_cycle_combination;
	if(cnt_for_cycle_combination == 7)
		cnt_for_cycle_combination = 0;
	for(i = 0; i < 19; i++)
	{
		led_config_logic[i].red = 0;
		led_config_logic[i].green = 0;
		led_config_logic[i].blue = 0;
	}
	if(!cnt_for_cycle_combination){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].green = 255;
			led_config_logic[i].red = 255;
			led_config_logic[i].blue = 255;
		}
	} else if(cnt_for_cycle_combination == 1){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].red = 255;
		}

	} else if(cnt_for_cycle_combination == 2){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].green = 255;
		}

	} else if(cnt_for_cycle_combination == 3){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].green = 201;
			led_config_logic[i].blue = 87;
		}

	} else if(cnt_for_cycle_combination == 4){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].blue = 230;
			led_config_logic[i].green = 224;
			led_config_logic[i].red = 176;
		}

	} else if(cnt_for_cycle_combination == 5){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].green = 255;
		}

	} else if(cnt_for_cycle_combination == 6){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].green = 192;
			led_config_logic[i].red = 255;
			led_config_logic[i].blue = 203;
		}

	}
}


// ------------------------- Display Mode Code 10 -------------------------
#define wait_adc_finish() while (!(ADC_CONTR & ADC_FLAG))   //等待ADC转换完成
#define adc_channel 0

void init_adc() {
    P1ASF |= 0x01;                      // P1(ch) -> P1 ^ 0
    low_at_bit(CLK_DIV, ADRJ);          // ADRJ = 0
    ADC_RES = 0;                        // high 8 bit       
    ADC_RESL = 0;                       // low 2 bit
    ADC_CONTR = ADC_POWER | ADC_SPEED | adc_channel;
}

static u8 GetADCResult()
{
    ADC_CONTR |= ADC_START;
    
    _nop_();                        
    _nop_();
    _nop_();
    _nop_();
    wait_adc_finish();              
    ADC_CONTR &= ~ADC_FLAG;         
    return ((ADC_RES << 2) | ADC_RESL);                
}


// 跟随音乐
static void music_sensor_driver() {
		led_config_change();
}

u8 cnt_for_cycle_music_sensor = 0;
static void music_sensor_driver_init() {
	u8 i;
	for(i = 0; i < 19; i++)
	{
		led_config_logic[i].red = 255;
		led_config_logic[i].green = 255;
		led_config_logic[i].blue = 255;
	}
	led_flicker.cycle_output = ON;
	led_flicker.cycle_enable = ENABLE;

	led_mode_logic = music_sensor;
	cnt_for_cycle_music_sensor = 0;
}

static void music_sensor_half_cycle_reached()
{
	u8 i;
	u8 adc_result_diff, adc_result;
	u8 adc_result_from_music = GetADCResult();
	if(adc_result_from_music > 0x7A)
	{
		adc_result_diff = adc_result_from_music - 0x7A;
		if(adc_result_diff < 3){
			adc_result = 50;
		}else if(adc_result_diff >= 3 && adc_result_diff < 10){
			adc_result = 100;
		}else if(adc_result_diff >= 10 && adc_result_diff < 13){
			adc_result = 125;
		}else if(adc_result_diff >= 13 && adc_result_diff < 17){
			adc_result = 150;
		}else if(adc_result_diff >= 17 && adc_result_diff < 20){
			adc_result = 175;
		}else if(adc_result_diff >= 20 && adc_result_diff < 23){
			adc_result = 200;
		}else if(adc_result_diff >= 23 && adc_result_diff < 26){
			adc_result = 225;
		}else if(adc_result_diff >= 26){
			adc_result = 250;
		}
	}
	else{
		adc_result_diff = 0x7A - adc_result_from_music;
		if(adc_result_diff < 3){
			adc_result = 50;
		}else if(adc_result_diff >= 3 && adc_result_diff < 10){
			adc_result = 25;
		}else if(adc_result_diff >= 10){
			adc_result = 0;
		}

	} 
	++ cnt_for_cycle_music_sensor;
	if(cnt_for_cycle_music_sensor == 7)
		cnt_for_cycle_music_sensor = 0;
	if(!cnt_for_cycle_music_sensor){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].red = adc_result;
		}

	} else if(cnt_for_cycle_music_sensor == 1){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].green = adc_result;
		}
	} else if(cnt_for_cycle_music_sensor == 2){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].blue = adc_result;
		}
	}  else if(cnt_for_cycle_music_sensor == 3){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].green = adc_result;
			led_config_logic[i].blue = adc_result;
		}
	} else if(cnt_for_cycle_music_sensor == 4){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].blue = adc_result;
			led_config_logic[i].red = adc_result;
		}
	} else if(cnt_for_cycle_music_sensor == 5){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].red = adc_result;
			led_config_logic[i].green = adc_result;
		}
	}else if(cnt_for_cycle_music_sensor == 6){
		for(i = 0; i < 19; i++)
		{
			led_config_logic[i].red = adc_result;
			led_config_logic[i].green = adc_result;
			led_config_logic[i].blue = adc_result;
		}
	}
}


void led_syc_color(u8 *param)
{
	u8 i;
	led_mode_logic = moon_light;
	led_flicker.cycle_output = ON;
	led_flicker.cycle_enable = DISABLE;
	for(i = 0; i < 19; i++)
	{
		led_config_logic[i].red = (param[1] - '0') * 100 +  (param[2] - '0') * 10 + (param[3] - '0');
		led_config_logic[i].green = (param[4] - '0') * 100 +  (param[5] - '0') * 10 + (param[6] - '0');
		led_config_logic[i].blue = (param[7] - '0') * 100 +  (param[8] - '0') * 10 + (param[9] - '0');
	}
	led_config_change();
}