#include "display.h"
#include "sync.h"
#include "utils.h"
#include "string.h"

volatile disp_t led_mode_set = led_display_off;
led_t xdata led_config_logic;
volatile toggle_t led_toggle_status = ON;

// ------------------------- display config declaring -------------------------

// Cycle are calculated based on 5ms timer
#define not_cycle_effective NULL
static code u16 const cycle_for_colorful[1] = {4};
u8 counter_for_color = 0;
u8 color_mark = 0;

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
static void steady_driver_init();
static void colorful_driver_init();
static void steady_from_app_driver_init();
// ------------------------- display_mode_driver declaring -------------------------
static void led_display_off_driver();
static void steady_driver();
static void colorful_driver();
static void steady_from_app_driver();
// ------------------------- half_cycle_reached declaring -------------------------

static void steady_half_cycle_reached();
static void colorful_half_cycle_reached();

static code const u16 *cycle_params[4] =
{
	not_cycle_effective, 						// led_display_off
	cycle_for_colorful, 						// colorful
	not_cycle_effective,						    // steady
	not_cycle_effective

};

// idata *display_driver -> detailed display_driver
static code driver_init_pt const driver_init[4] =
{
	led_display_off_driver_init, 		// led_display_off
	colorful_driver_init, 				// colorful
	steady_driver_init,					// steady
	steady_from_app_driver_init
};

static code half_cycle_reached_pt const half_cycle_callback[4] = 
{
	NULL, 										// led_display_off
	colorful_half_cycle_reached, 				// colorful	
	NULL,										// steady
	NULL
};

static code display_driver_pt const display_driver[4] = 
{
	led_display_off_driver, 			// led_display_off
	colorful_driver, 					// colorful_driver
	steady_driver,						// steady
	steady_from_app_driver
};

static void ctrl_color_changed(const u8 red, const u8 green, const u8 blue) {
	led_config_set.red = red;
	led_config_set.green = green;
	led_config_set.blue = blue;
}

// ------------------------- public -------------------------
// ONLY CALL BY main_isr
// CAUTIONS!  DON't CALL THIS
void display_startup_init() {
	led_config_init(&led_config_set);
	led_config_init(&led_config_logic);
	led_display_off_driver_init();
}

void led_display_task() {
	led_cycle_based_adjust();
	prepare_next_time_led_config();
}


// spi-> ctrl_mode_changed. Callback() & internal change
static void led_mode_changed()  {
	driver_init_pt led_mode_driver_init = driver_init[led_mode_set];
	if (led_mode_driver_init)
		led_mode_driver_init();
	led_flicker.counter_for_cycle = 0;
}

void ctrl_mode_changed(const u8 mode_code) {
	led_mode_set = (disp_t)mode_code;
	led_mode_changed();
} 


// ------------------------- private -------------------------
static void prepare_next_time_led_config() {
	led_color_8bit(&led_config_logic);
}

static void led_cycle_based_adjust() {
	if (led_flicker.cycle_enable) {
		if (led_flicker.counter_for_cycle >= (cycle_params[led_mode_set][0] >> 1))
			half_cycle_reached();
	} else {
		led_flicker.counter_for_cycle = 0;
	}
}


static void half_cycle_reached() {
	if (half_cycle_callback[led_mode_set])
		half_cycle_callback[led_mode_set]();
	led_flicker.counter_for_cycle = 0;
}

// ------------------------- Display Mode Code 0--------------------------
static void led_display_off_driver(void) {
	led_config_logic.red = 0;
	led_config_logic.green = 0;
	led_config_logic.blue = 0;
}

static void led_display_off_driver_init() {
	led_flicker.cycle_output = OFF;
	led_flicker.cycle_enable = DISABLE;
	led_config_logic.red = 0;
	led_config_logic.green = 0;
	led_config_logic.blue = 0;
}

// ------------------------- Display Mode Code 1 --------------------------

static void steady_driver(void) {

}

static void steady_driver_init() {
	led_flicker.cycle_output = ON;
	led_flicker.cycle_enable = DISABLE;
}



// ------------------------- Display Mode Code 1 --------------------------

static void steady_from_app_driver(void) {

}

static void steady_from_app_driver_init() {
	led_flicker.cycle_output = ON;
	led_flicker.cycle_enable = DISABLE;
	led_config_cpy(&led_config_logic, &led_config_set);
}

// ------------------------- Display Mode Code 8 --------------------------
static void colorful_driver(void) {


}

static void colorful_driver_init() {
	led_flicker.cycle_output = ON;
	led_flicker.cycle_enable = ENABLE;
	led_config_logic.red = 251;
	led_config_logic.green = 1;
	led_config_logic.blue = 1;
	counter_for_color = 0;
	color_mark = 0;
}

static void colorful_half_cycle_reached() {
	// start with ON
	counter_for_color ++;
	if(counter_for_color >= 250)
	{
		counter_for_color = 0;
		color_mark ++;
		if(color_mark >= 6)
			color_mark = 0;
	} 
	if(!color_mark)
	{
		led_config_logic.green ++;
	} else if(color_mark == 1){
		led_config_logic.red --;
	} else if(color_mark == 2){
		led_config_logic.blue ++;
	} else if(color_mark == 3){
		led_config_logic.green --;
	} else if(color_mark == 4){
		led_config_logic.red ++;
	} else if(color_mark == 5){
		led_config_logic.blue --;
	} 
}


void led_switch_on_off(u8 led_on)
{
	if(led_on)
	{
		led_mode_set = colorful;
		ctrl_mode_changed(led_mode_set);

	}
	else
	{
		led_mode_set = led_display_off;
		ctrl_mode_changed(led_mode_set);
	}
}

void led_syc_color(u8 *param)
{
	u8 i = 0;
	u8 param_init[6];
	led_mode_set = steady_from_app;
	for(i = 4; i < 10; i++)
	{
		if (param[i]>='0' && param[i]>='0') 
			param_init[i-4] = param[i] - '0';
		else if(param[i]>='a' && param[i]<='f') 
			param_init[i-4] = param[i] - 'a' + 10;
	}
	led_config_set.red = param_init[0] << 8 |  param_init[1];
	led_config_set.green = param_init[2] << 8 |  param_init[3];
	led_config_set.blue = param_init[4] << 8 |  param_init[5];;
	ctrl_mode_changed(led_mode_set);
}