#include "shared.h"
#include "display.h"
#include "sync.h"
#include "def.h"
#include "intrins.h"
#include "adc.h"
#include "uart.h"
// ------------------------- display config declaring -------------------------
static code disp_t const
next_display_mode[12] = {sequential, slow_glo, chasing_flash, slow_fade, twinkle_flash, stacking_flash,
	flash_and_chase, random_flashing, pulsing, sequential_flashing, bi_directional_storbing, in_wave};

// combination start from second disp_t
#define next_display_mode_from 	1

// TODO 60Hz is ok, 50Hz is not
static code u16 const
phase_delay_for_60Hz[10] = {0x2666, 0x2533, 0x2280, 0x201A, 0x1CB3, 0x1B26, 0x18C0, 0x1633, 0x13CD, 0x0F00};
// Cycle are calculated based on 5ms PCA timer -> sync.h
// 2
static code u16 const 
cycle_for_in_wave[4][11]= {
	// 最后一位是半个周期的长度
	{10, 5, 5, 5, 5, 10, 5, 5, 5, 5, 30},
	{30, 15, 15, 15, 15, 30, 15, 15, 15, 15, 90},
	{20, 10, 10, 10, 10, 20, 10, 10, 10, 10, 60},
	{40, 20, 20, 20, 20, 40, 20, 20, 20, 20, 120}
};
// 3
static code u16 const 
cycle_for_sequential[5][6] = {
	{100, 50, 50, 50, 50, 100},
	{80, 40, 40, 40, 40, 80},
	{60, 30, 30, 30, 30, 60},
	{40, 20, 20, 20, 20, 40},
	{20, 10, 10, 10, 10, 20}
};
static u8 cycle_counter_for_sequential_per_interval[5] = {2,4,6,8,10};
static u8 sequential_cycle_counter = 0;
// 4
static code u16 const 
cycle_for_slow_glo[4][11] = {
	{1200, 400, 400, 400, 400, 400, 400, 400, 400, 1200, 400},
	{600, 200, 200, 200, 200, 200, 200, 200, 200, 600, 200},
	{300, 100, 100, 100, 100, 100, 100, 100, 100, 300, 200},
	{150, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50}
};
// 5
static code u16 const 
cycle_for_chasing_flash[4][16] = {
	{20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20},
	{20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20},
	{20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20},
	{20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20}
};
static u8 cycle_counter_for_chasing_flash_per_interval[4] = {4,1,4,1};
static u8 chasing_flash_cycle_counter = 0;
// 6
static code u16 const 
cycle_for_slow_fade[16] = {1600, 1600, 1600, 1600, 800, 800, 800, 800, 500, 500, 500, 500, 400, 400, 400, 400};
// 7
static code u16 const 
cycle_for_twinkle_flash[16] = {20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20};


// 8
static code u16 const 
cycle_for_stacking_flash[11] = {67, 60, 53, 46, 39, 100, 28, 22, 16, 11, 5};
// 9
static code u16 const 
cycle_for_flash_and_chase[11] = {67, 60, 53, 46, 39, 100, 28, 22, 16, 11, 5};
// 10
static code u16 const 
cycle_for_random_flashing[11] = {67, 60, 53, 46, 39, 100, 28, 22, 16, 11, 5};

// 11
static code u16 const 
cycle_for_pulsing[11] = {367, 333, 300, 267, 233, 200, 167, 133, 100, 67, 33};
// 12
static code u16 const 
cycle_for_sequential_flashing[11] = {67, 60, 53, 46, 39, 100, 28, 22, 16, 11, 5};

// 13
static code u16 const
cycle_for_bi_directional_storbing[11] = {51, 47, 42, 37, 33, 27, 23, 19, 14, 9, 5};

// 14
static code u16 const 
cycle_for_sensor[12] = {200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200};

#define cycle_for_not_effective_default NULL

#define BASE_AVG 0x50
// Sensor mode calculation based
// Lumins level 3 - 10
static code u8 const 
diff_avg_range_1[8] = {1, 2, 3, 4, 5, 6, 7, 8};
static code u8 const   
diff_avg_range_2[8] = {1, 3, 5, 8, 10, 11, 13, 14};
static code u8 const   
diff_avg_range_3[8] = {4, 7, 11, 15, 20, 26, 31, 35};
static code u8 const   
diff_avg_range_4[8] = {4, 7, 11, 15, 18, 24, 28, 31};

// Lumins level 2 for max_of_period_max < max_avg
#define below_avg_threshhold_for_peace		8
// Lumins level 1 for each range
static code u8 const   
below_avg_threshold[4] = {9, 10, 11, 12};
static code u8 const   
avg_range[4] = {0x00, 0x20, 0x25, 0x28};

// ------------------------- display private params -------------------------
// idata *cycle_param -> cycle_for_detailed_display_mode in code
static const u16 code * data cycle_params;
// point to  , self can be modify
static const u16 code * data current_phase_delay;
// idata *display_driver -> detailed display_driver
static display_driver_pt display_driver;
static cycle_reached_pt cycle_reached;
// lumins_level need to be fine-grained
static BOOL lumins_level_fine_override = NO; 
// deviation based on lower lumins_level_t in both RISE | FALL phase
//static u8 lumins_level_deviation = 0;
static u8 lumins_level_deviation[3] = {0};
// mode 完成次数
static u8 counter_for_current_cycle = 0;
static u8 counter_for_minute = 6;


#define fade_in_out_fine_grained_level 10
//static bit fade_in_out_flag = RISE;
static u8 fade_in_out_flag[3] = {RISE, RISE, RISE};
static bit dim_in_out_flag = FULL;
#define lumins_level_min_deviation_interval_for_60Hz  120
static u8 current_interval = 0;
static u8 slow_glo_counter = 0;
static u8 in_wave_counter = 0;
static const u8 code * data diff;

#define AVG_BUFFER_SIZE_BIT 4
#define AVG_BUFFER_SIZE 	(1 << AVG_BUFFER_SIZE_BIT)
#define PERIOD_MAX_SIZE 	4
static xdata u16 avg_buffer[AVG_BUFFER_SIZE];
static u16 period_max_buffer[PERIOD_MAX_SIZE];
static u16 current_adc_max, max_sum = BASE_AVG << AVG_BUFFER_SIZE_BIT, max_avg = BASE_AVG;
static u8 max_avg_index = 0, period_max_index = 0;
static toggle_t toggle_stat_last_for_sensor_prevent_inst_blink = ON;
static BOOL sensor_initializing = NO;

u8 sensor_counter = 1; 

// ------------------------- marco functions -------------------------

#define reverse_dim_in_out_flag() dim_in_out_flag = ~dim_in_out_flag 
#define sync_up_toggle_stat()	toggle_stat_last_for_sensor_prevent_inst_blink = toggle_status
#define index_of(x) x - 1
//#define set_phase_timer(x) TL0 = x; TH0 = x >> 8
#define lumins_level_fine_override_should_change(target) lumins_level_fine_override = (target == steady_on || target == sensor || target == bi_directional_storbing || target == stacking_flash || target == random_flashing || target == pulsing || target == sequential_flashing) ? NO : YES
#define sensor_should_reinit_from_awake()	( toggle_status == ON && toggle_stat_last_for_sensor_prevent_inst_blink == OFF)
#define sensor_initializing_finished()	(max_avg_index == (AVG_BUFFER_SIZE >> 2))
// ------------------------- private declaring -------------------------
//static void half_cycle_reached();
static u16 recalibrate_phase_delay(lumins_level_t lumins,   u8 deviation);
static u16 find_period_max_in_buffer();

// ------------------------- display_mode_init declaring -------------------------
static void combination_driver_init();
static void in_wave_driver_init();
static void sequential_driver_init();
static void slow_glo_driver_init();
static void chasing_flash_driver_init();
static void slow_fade_driver_init();
static void twinkle_flash_driver_init();
static void stacking_flash_driver_init();
static void flash_and_chase_driver_init();
static void random_flashing_driver_init();
static void pulsing_driver_init();
static void sequential_flashing_driver_init();
static void bi_directional_storbing_driver_init();
static void steady_on_driver_init();
static void sensor_driver_init();

// ------------------------- display_mode_driver declaring -------------------------
//static void combination_driver();
static void in_wave_driver();
static void sequential_driver();
static void slow_glo_driver();
static void chasing_flash_driver();
static void slow_fade_driver();
static void twinkle_flash_driver();
static void stacking_flash_driver();
static void flash_and_chase_driver();
static void random_flashing_driver();
static void pulsing_driver();
static void sequential_flashing_driver();
static void bi_directional_storbing_driver();
static void steady_on_driver();
static void sensor_driver();

// ------------------------- display_mode_cycle_reached declaring -------------------------
static void cycle_reached_in_wave(u16);
static void cycle_reached_sequential(u16);
static void cycle_reached_slow_glo(u16);
static void cycle_reached_chasing_flash(u16);
static void cycle_reached_slow_fade(u16);
static void cycle_reached_twinkle_flash(u16);
static void cycle_reached_stacking_flash(u16);
static void cycle_reached_flash_and_chase(u16);
static void cycle_reached_random_flashing(u16);
static void cycle_reached_pulsing(u16);
static void cycle_reached_sequential_flashing(u16);
static void cycle_reached_bi_directional_storbing(u16);
//static void cycle_reached_steady_burn(u16);
// static void cycle_reached_sensor(u16);


// ------------------------- public -------------------------
// ONLY CALL BY main_isr
// CAUTIONS!  DON't CALL THIS
void display_startup_init() {
	_nop_();							// 1. no need to call steady_driver_init
	display_driver = steady_on_driver;	// 2. dispatch display_driver
	cycle_params = cycle_for_not_effective_default; // 3. dispatch cycle_params
//	cycle_reached = cycle_reached_chasing_flash;
	_nop_();							// 4. lumins_level_fine_override set by default to NO
	_nop_();							// 5. counters are init by sync.c
	current_phase_delay = phase_delay_for_60Hz;	// 6. reset current phase_delay
//	display_mode_set_changed();
	sensor_driver_init();
}

void
prepare_next_display_params() {
	if(!P12)
	{
		adc_should_sampling = NO;
		display_driver();
	}
	else
	{
		adc_should_sampling = YES;
		sensor_driver();
	}
	// preserve toggle_status for next round
	sync_up_toggle_stat();
}

void
ctrl_toggle_status_response(const toggle_t prev) {
	if (toggle_status != prev) {
		_nop_();
	}
}

void
ctrl_display_mode_response(const disp_t prev) {
	if (display_mode_set != prev) {
		adc_disable();
		display_mode_set_changed();
	}
}

void
toggle_once() {
	lumins_level_t lumins[3];
	u16 current_counter;
	lumins_level_logic[0] = lumins_level_logic_prepared[0];
	lumins_level_logic[1] = lumins_level_logic_prepared[1];
	lumins_level_logic[2] = lumins_level_logic_prepared[2];
	lumins[0] = lumins_level_logic[0];
	lumins[1] = lumins_level_logic[1];
	lumins[2] = lumins_level_logic[2];
	calibrated_phase_delay[0] = 0;
	calibrated_phase_delay[1] = 0;
	calibrated_phase_delay[2] = 0;	
	if (lumins[0] && cycle_output[0])
		calibrated_phase_delay[0] = current_phase_delay[lumins[0] - 1];
	if (lumins[1] && cycle_output[1])
		calibrated_phase_delay[1] = current_phase_delay[lumins[1] - 1];
	if (lumins[2] && cycle_output[2])
		calibrated_phase_delay[2] = current_phase_delay[lumins[2] - 1];
	current_counter =  (CH << 8) | CL;

	if (!lumins_level_fine_override || (lumins_level_fine_override && P12)) {
		
		CCPA_tmp[0] = current_counter + calibrated_phase_delay[0];
        CCAP0L = CCPA_tmp[0] ;
        CCAP0H = CCPA_tmp[0] >> 8;
		CCPA_tmp[1] = current_counter + calibrated_phase_delay[1];
        CCAP1L = CCPA_tmp[1] ;
        CCAP1H = CCPA_tmp[1] >> 8;
        CCPA_tmp[2] = current_counter + calibrated_phase_delay[2];
        CCAP2L = CCPA_tmp[2] ;
        CCAP2H = CCPA_tmp[2] >> 8;

	} 
	else 
	{ 
		if(cycle_output[0])
			calibrated_phase_delay[0] = recalibrate_phase_delay(lumins[0], lumins_level_deviation[0]);
		else
			calibrated_phase_delay[0] = 0;
		if(cycle_output[1])
      		calibrated_phase_delay[1] = recalibrate_phase_delay(lumins[1], lumins_level_deviation[1]);
		else
			calibrated_phase_delay[1] = 0;
		if(cycle_output[2])
      		calibrated_phase_delay[2] = recalibrate_phase_delay(lumins[2], lumins_level_deviation[2]);
		else
			calibrated_phase_delay[2] = 0;

		CCPA_tmp[0] = current_counter + calibrated_phase_delay[0];
    	CCAP0L = CCPA_tmp[0] ;
    	CCAP0H = CCPA_tmp[0] >> 8;
        
		CCPA_tmp[1] = current_counter + calibrated_phase_delay[1];
    	CCAP1L = CCPA_tmp[1] ;
    	CCAP1H = CCPA_tmp[1] >> 8;
		
		CCPA_tmp[2] = current_counter + calibrated_phase_delay[2];
    	CCAP2L = CCPA_tmp[2] ;
    	CCAP2H = CCPA_tmp[2] >> 8;

	}
	phase_ctrl_enable();
}

void 
cycle_based_adjust(const u16 cycle) {
	if(display_mode_set == steady_on)
	{
		reset_counter_for_cycle();
	}
	else
	{
		cycle_reached(cycle);
	}

}

void 
display_mode_set_changed() {
	// DON't trig display_mode_logic_changed()
	// -> sync.h
	cycle_ctrl_disable();
	display_mode_logic = display_mode_set;
	// 1. driver init
	// 2. display_driver -> detailed
	// 3. cycle_params -> detailed
	switch (display_mode_set) {
		case combination:
			combination_driver_init();
			// display_mode_logic will never be combination
			// don't set display driver -> combination driver
			// don't set cycle_params -> any
			break;
		case in_wave: 
			in_wave_driver_init();
			display_driver = in_wave_driver;
			cycle_params = cycle_for_in_wave[0];
			cycle_reached = cycle_reached_in_wave;
			break;
		case sequential: 
			sequential_driver_init();
			display_driver = sequential_driver;
			cycle_params = cycle_for_sequential[0];
			cycle_reached = cycle_reached_sequential;
			break;
		case slow_glo: 
			slow_glo_driver_init();
			display_driver = slow_glo_driver;
			cycle_params = cycle_for_slow_glo[0];
			cycle_reached = cycle_reached_slow_glo;
			break;
		case chasing_flash: 
			chasing_flash_driver_init();
			display_driver = chasing_flash_driver;
			cycle_params = cycle_for_chasing_flash[0];
			cycle_reached = cycle_reached_chasing_flash;
			break;
		case slow_fade: 
			slow_fade_driver_init();
			display_driver = slow_fade_driver;
			cycle_params = cycle_for_slow_fade;
			cycle_reached = cycle_reached_slow_fade;
			break;
		case twinkle_flash: 
			twinkle_flash_driver_init();
			display_driver = twinkle_flash_driver;
			cycle_params = cycle_for_twinkle_flash;
			cycle_reached = cycle_reached_twinkle_flash;
			break;
		case stacking_flash: 
			stacking_flash_driver_init();
			display_driver = stacking_flash_driver;
			cycle_params = cycle_for_stacking_flash;
			cycle_reached = cycle_reached_stacking_flash;
			break;
		case flash_and_chase: 
			flash_and_chase_driver_init();
			display_driver = flash_and_chase_driver;
			cycle_params = cycle_for_flash_and_chase;
			cycle_reached = cycle_reached_flash_and_chase;
			break;
		case random_flashing: 
			random_flashing_driver_init();
			display_driver = random_flashing_driver;
			cycle_params = cycle_for_random_flashing;
			cycle_reached = cycle_reached_random_flashing;
			break;
		case pulsing: 
			pulsing_driver_init();
			display_driver = pulsing_driver;
			cycle_params = cycle_for_pulsing;
			cycle_reached = cycle_reached_pulsing;
			break;
		case sequential_flashing: 
			sequential_flashing_driver_init();
			display_driver = sequential_flashing_driver;
			cycle_params = cycle_for_sequential_flashing;
			cycle_reached = cycle_reached_sequential_flashing;
			break;
		case bi_directional_storbing: 
			bi_directional_storbing_driver_init();
			display_driver = bi_directional_storbing_driver;
			cycle_params = cycle_for_bi_directional_storbing;
			cycle_reached = cycle_reached_bi_directional_storbing;
			break;
		case steady_on: 
			steady_on_driver_init();
			display_driver = steady_on_driver;
			cycle_params = cycle_for_not_effective_default;
			break;		
		case sensor:
			// sensor_driver_init();
			// display_driver = sensor_driver;
			// cycle_params = cycle_for_sensor;
			// cycle_reached = cycle_reached_sensor;
			break;
		default:
			break; 
	}

	// override flag -> YES:	(slow)fade-in-out
	// override flag -> NO: 	else
	lumins_level_fine_override_should_change(display_mode_logic);
	reset_all_counters();
	// -> sync.h
 	cycle_ctrl_enable();
}

// called by pca_isr (combination rotation)
// called by com_isr (combination init)
void 
display_mode_logic_changed() {
	// 1. driver init
	// 2. display_driver -> detailed
	// 3. cycle_params -> detailed
	// -> sync.h
	//cycle_ctrl_disable();
	switch (display_mode_logic) {

		case combination:
			// display_mode_logic will never be combination
			break;
		case in_wave: 
			in_wave_driver_init();
			display_driver = in_wave_driver;
			cycle_params = cycle_for_in_wave[0];
			cycle_reached = cycle_reached_in_wave;
			break;
		case sequential: 
			sequential_driver_init();
			display_driver = sequential_driver;
			cycle_params = cycle_for_sequential[0];
			cycle_reached = cycle_reached_sequential;
			break;
		case slow_glo: 
			slow_glo_driver_init();
			display_driver = slow_glo_driver;
			cycle_params = cycle_for_slow_glo[0];
			cycle_reached = cycle_reached_slow_glo;
			break;
		case chasing_flash: 
			chasing_flash_driver_init();
			display_driver = chasing_flash_driver;
			cycle_params = cycle_for_chasing_flash[0];
			cycle_reached = cycle_reached_chasing_flash;
			break;
		case slow_fade: 
			slow_fade_driver_init();
			display_driver = slow_fade_driver;
			cycle_params = cycle_for_slow_fade;
			cycle_reached = cycle_reached_slow_fade;
			break;
		case twinkle_flash: 
			twinkle_flash_driver_init();
			display_driver = twinkle_flash_driver;
			cycle_params = cycle_for_twinkle_flash;
			cycle_reached = cycle_reached_twinkle_flash;
			break;
		case stacking_flash: 
			stacking_flash_driver_init();
			display_driver = stacking_flash_driver;
			cycle_params = cycle_for_stacking_flash;
			cycle_reached = cycle_reached_stacking_flash;
			break;
		case flash_and_chase: 
			flash_and_chase_driver_init();
			display_driver = flash_and_chase_driver;
			cycle_params = cycle_for_flash_and_chase;
			cycle_reached = cycle_reached_flash_and_chase;
			break;
		case random_flashing: 
			random_flashing_driver_init();
			display_driver = random_flashing_driver;
			cycle_params = cycle_for_random_flashing;
			cycle_reached = cycle_reached_random_flashing;
			break;
		case pulsing: 
			pulsing_driver_init();
			display_driver = pulsing_driver;
			cycle_params = cycle_for_pulsing;
			cycle_reached = cycle_reached_pulsing;
			break;
		case sequential_flashing: 
			sequential_flashing_driver_init();
			display_driver = sequential_flashing_driver;
			cycle_params = cycle_for_sequential_flashing;
			cycle_reached = cycle_reached_sequential_flashing;
			break;
		case bi_directional_storbing: 
			bi_directional_storbing_driver_init();
			display_driver = bi_directional_storbing_driver;
			cycle_params = cycle_for_bi_directional_storbing;
			cycle_reached = cycle_reached_bi_directional_storbing;
			break;
		case steady_on: 
			// steady_burn_driver_init();
			// display_driver = steady_burn_driver;
			// cycle_params = cycle_for_not_effective_default;
			break;		
		case sensor:
			// com_isr -> display_mode_set := sensor -> sensor_init
			// no other routine direct called 
			break;
		default:
			break; 
	}
	reset_counter_for_cycle();
	// override flag -> YES:	(slow)fade-in-out
	// override flag -> NO: 	else
	lumins_level_fine_override_should_change(display_mode_logic);
	// dont't reset all counters
	//cycle_ctrl_enable();
}

void
skip_to_next_display_if_should() {
	if(display_mode_set == in_wave || display_mode_logic == in_wave)
	{
		in_wave_counter ++;
		if(in_wave_counter == 4)
		{
			in_wave_counter = 0;
		}
		cycle_params = cycle_for_in_wave[in_wave_counter];
	}
	if(display_mode_set == combination)
	{
		--counter_for_minute;
		if(!counter_for_minute)
		{
			counter_for_minute = 6; 
			display_mode_logic = next_display_mode[display_mode_logic - next_display_mode_from];
			display_mode_logic_changed();
		}

	}

}
/*
*	params <lumins_level_t> lumins: lumins_level_min to lumins_level_max
*	params <u8> deviation: 0 - 9
*	
*/
static u16 
recalibrate_phase_delay(const lumins_level_t lumins, const u8 deviation) {
	if (lumins == lumins_level_max) {
	//	return current_phase_delay[index_of(lumins_level_max)];
		return current_phase_delay[index_of(lumins_level_max)];
	} else if (lumins == lumins_level_min) {
		u16 phase_delay_adjust = (fade_in_out_fine_grained_level - deviation) * lumins_level_min_deviation_interval_for_60Hz;
		// lumins_level_min ------ 1 ------ 2 ------ ... ------ 9 ------- lumins_level_min + 1
		return current_phase_delay[0] + phase_delay_adjust;
	} else {
		u16 phase_delay_delta =current_phase_delay[index_of(lumins)] - current_phase_delay[lumins];
		u16 phase_delay_interval = phase_delay_delta / fade_in_out_fine_grained_level;
		return current_phase_delay[index_of(lumins)] - phase_delay_interval * deviation;
	}
}

static u16 find_period_max_in_buffer() {
	u16 current_max = 0;
	u8 i;
	for (i = PERIOD_MAX_SIZE; i; ) {
		if (current_max < period_max_buffer[--i])
			current_max = period_max_buffer[i];
	}
	return current_max;
}

// 0
static void
combination_driver_init() {
	cycle_output[0] = ON;
	cycle_output[1] = ON;
	cycle_output[2] = ON;
	display_mode_logic = in_wave;
	counter_for_minute = 6;
	// cycle_params = cycle_for_flash1_directional;		// 3. dispatch cycle_params
	// cycle_reached = cycle_reached_flash1_directional;
	display_mode_logic_changed();
}

// 1
// in_wave
static void 
in_wave_driver(void) {
	u8 i,current_progress[3];
	u16 current_counter = counter_for_cycle;
	u16 progress = (current_counter * 100UL) / cycle_params[10];
	if(current_interval == 0){
		current_progress[0] = 100 - progress;
		current_progress[1] = 66 - progress;
		current_progress[2] = 33 - progress;
	}else if(current_interval == 1){
		current_progress[0] = 66 - progress;
		current_progress[1] = 33 - progress;
		current_progress[2] = progress;
	}else if(current_interval == 2){
		current_progress[0] = 50 - progress;
		current_progress[1] = 17 - progress;
		current_progress[2] = 17 + progress;
	}else if(current_interval == 3){
		current_progress[0] = 33 - progress;
		current_progress[1] = progress;
		current_progress[2] = 33 + progress;
	}else if(current_interval == 4){
		current_progress[0] = 17 - progress;
		current_progress[1] = progress + 17;
		current_progress[2] = progress + 50;
	}else if(current_interval == 5){
		current_progress[0] = progress;
		current_progress[1] = progress + 33;
		current_progress[2] = progress + 66;
	}else if(current_interval == 6){
		current_progress[0] = progress + 33;
		current_progress[1] = progress + 66;
		current_progress[2] = 100 - progress;
	}else if(current_interval == 7){
		current_progress[0] = progress + 50;
		current_progress[1] = progress + 83;
		current_progress[2] = 83 - progress;
	}else if(current_interval == 8){
		current_progress[0] = progress + 66;
		current_progress[1] = 100 - progress;
		current_progress[2] = 66 - progress;
	}else if(current_interval == 9){
		current_progress[0] = progress + 83;
		current_progress[1] = 83 - progress;
		current_progress[2] = 50 - progress;
	}
	for (i = 0; i < 3; ++i)
	{
		lumins_level_logic_prepared[i] = current_progress[i] / fade_in_out_fine_grained_level;
		lumins_level_deviation[i] = current_progress[i] - lumins_level_logic_prepared[i] * fade_in_out_fine_grained_level;
	}
}

static void
in_wave_driver_init() {
	cycle_output[0] = ON;
	cycle_output[1] = ON;
	cycle_output[2] = ON;
	current_interval = 0;
	in_wave_counter = 0;
}


static void 
cycle_reached_in_wave(u16 cycle) {
	if(cycle >= cycle_params[current_interval])
	{
		++ 	current_interval;
		if(current_interval == 10)
			current_interval = 0;
		reset_counter_for_cycle();
	}
}

// 2
// sequential
static void 
sequential_driver(void) {
	u8 i,current_progress[3];
	u16 current_counter = counter_for_cycle;
	u16 progress = (current_counter * 100UL) / cycle_params[0];
	if(current_interval == 0){
		current_progress[0] = 0;
		current_progress[1] = 0;
		current_progress[2] = progress;
	}else if(current_interval == 1){
		current_progress[0] = 0;
		current_progress[1] = progress;
		current_progress[2] = 100 - progress;
	}else if(current_interval == 2){
		current_progress[0] = 0;
		current_progress[1] = 50 + progress;
		current_progress[2] = 50 - progress;
	}else if(current_interval == 3){
		current_progress[0] = progress;
		current_progress[1] = 100 - progress;
		current_progress[2] = 0;
	}else if(current_interval == 4){
		current_progress[0] = 50 + progress;
		current_progress[1] = 50 - progress;
		current_progress[2] = 0;
	}else if(current_interval == 5){
		current_progress[0] = 100 - progress;
		current_progress[1] = 0;
		current_progress[2] = 0;
	}
	for (i = 0; i < 3; ++i)
	{
		lumins_level_logic_prepared[i] = current_progress[i] / fade_in_out_fine_grained_level;
		lumins_level_deviation[i] = current_progress[i] - lumins_level_logic_prepared[i] * fade_in_out_fine_grained_level;
	}
}

static void
sequential_driver_init() {
	cycle_output[0] = OFF;
	cycle_output[1] = OFF;
	cycle_output[2] = ON;
	current_interval = 0;
	counter_for_current_cycle = 0;
	sequential_cycle_counter = cycle_counter_for_sequential_per_interval[counter_for_current_cycle];
}

static void 
cycle_reached_sequential(u16 cycle) 
{
	if(cycle >= cycle_params[current_interval])
	{
		if(current_interval == 0)
		{
			cycle_output[0] = OFF;
			cycle_output[1] = ON;
			cycle_output[2] = ON;
		}
		if(current_interval == 2)
		{
			cycle_output[0] = ON;
			cycle_output[1] = ON;
			cycle_output[2] = OFF;
		}
		if(current_interval == 4)
		{
			cycle_output[0] = ON;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		if(current_interval == 5)
		{
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = ON;
		}
		++ 	current_interval;
		if(current_interval == 6)
		{
			current_interval = 0;
			-- sequential_cycle_counter;
			if(sequential_cycle_counter == 0)
			{
				counter_for_current_cycle++;
				if(counter_for_current_cycle == 5)
				{
					counter_for_current_cycle = 0;
				}
				cycle_params = cycle_for_sequential[counter_for_current_cycle]; 
				sequential_cycle_counter = cycle_counter_for_sequential_per_interval[counter_for_current_cycle];
			}
		}
		reset_counter_for_cycle();
	}
}


// 3
// slow_glo
static void 
slow_glo_driver(void) {
	u8 i,current_progress[3];
	u16 current_counter = counter_for_cycle;
	u16 progress = (current_counter * 100UL) / cycle_params[0];
	if(current_interval == 0){
		current_progress[0] = progress;
		current_progress[1] = 0;
		current_progress[2] = 0;
	}else if(current_interval == 1){
		current_progress[0] = 100 - progress;
		current_progress[1] = 0;
		current_progress[2] = 0;
	}else if(current_interval == 2){
		current_progress[0] = 66 - progress;
		current_progress[1] = progress;
		current_progress[2] = 0;
	}else if(current_interval == 3){
		current_progress[0] = 33 - progress;
		current_progress[1] = progress + 33;
		current_progress[2] = 0;
	}else if(current_interval == 4){
		current_progress[0] = 0;
		current_progress[1] = progress + 66;
		current_progress[2] = 0;
	}else if(current_interval == 5){
		current_progress[0] = 0;
		current_progress[1] = 100 - progress;
		current_progress[2] = 0;
	}else if(current_interval == 6){
		current_progress[0] = 0;
		current_progress[1] = 66 - progress;
		current_progress[2] = progress;
	}else if(current_interval == 7){
		current_progress[0] = 0;
		current_progress[1] = 33 - progress;
		current_progress[2] = progress + 33;
	}else if(current_interval == 8){
		current_progress[0] = 0;
		current_progress[1] = 0;
		current_progress[2] = progress + 66;
	}else if(current_interval == 9){
		current_progress[0] = 0;
		current_progress[1] = 0;
		current_progress[2] = 100 - progress;
	}else if(current_interval == 10){
		current_progress[0] = 0;
		current_progress[1] = 0;
		current_progress[2] = 0;
	}
	for (i = 0; i < 3; ++i)
	{
		lumins_level_logic_prepared[i] = current_progress[i] / fade_in_out_fine_grained_level;
		lumins_level_deviation[i] = current_progress[i] - lumins_level_logic_prepared[i] * fade_in_out_fine_grained_level;
	}
}

static void
slow_glo_driver_init() {
	cycle_output[0] = ON;
	cycle_output[1] = OFF;
	cycle_output[2] = OFF;
	current_interval = 0;
	slow_glo_counter = 0;
}


static void 
cycle_reached_slow_glo(u16 cycle) {
	if(cycle >= cycle_params[current_interval])
	{
		if(current_interval == 1)
		{
			cycle_output[0] = ON;
			cycle_output[1] = ON;
			cycle_output[2] = OFF;
		}
		if(current_interval == 3)
		{
			cycle_output[0] = OFF;
			cycle_output[1] = ON;
			cycle_output[2] = OFF;
		}
		if(current_interval == 5)
		{
			cycle_output[0] = OFF;
			cycle_output[1] = ON;
			cycle_output[2] = ON;
		}
		if(current_interval == 7)
		{
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = ON;
		}
		if(current_interval == 9)
		{
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		if(current_interval == 10)
		{
			cycle_output[0] = ON;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		++ 	current_interval;
		if(current_interval == 11)
		{
			current_interval = 0;
			++ slow_glo_counter;
			if(slow_glo_counter == 4)
			{
				slow_glo_counter = 0;
			}
			cycle_params = cycle_for_slow_glo[slow_glo_counter];
		}
		reset_counter_for_cycle();
	}
}

// 4
// chasing_flash
static void 
chasing_flash_driver(void) {
	u8 i;
	u16 current_counter = counter_for_cycle;
	for(i = 0; i < 3; i++)
	{
		u16 progress = (current_counter * 100UL) / (cycle_for_twinkle_flash[current_interval]) ;
		u16 reversed_cycle_rate_of_progress = 100 - progress;

		if (fade_in_out_flag[i] == RISE) {
		// 0 -> 10
			lumins_level_logic_prepared[i] = progress / fade_in_out_fine_grained_level;
			lumins_level_deviation[i] = progress - lumins_level_logic_prepared[i] * fade_in_out_fine_grained_level;
		} else { // FALL
		// 10 -> 0
			lumins_level_logic_prepared[i] = reversed_cycle_rate_of_progress / fade_in_out_fine_grained_level;
			lumins_level_deviation[i] = reversed_cycle_rate_of_progress - lumins_level_logic_prepared[i] * fade_in_out_fine_grained_level;
		}
	}
}

static void
chasing_flash_driver_init() {
	cycle_output[0] = ON;
	cycle_output[1] = OFF;
	cycle_output[2] = OFF;
	fade_in_out_flag[0] = RISE;
	fade_in_out_flag[1] = RISE;
	fade_in_out_flag[2] = RISE;	
	lumins_level_logic_prepared[0] = lumins_level_min;
	lumins_level_logic_prepared[1] = lumins_level_min;
	lumins_level_logic_prepared[2] = lumins_level_min;
	current_interval = 0;
	counter_for_current_cycle = 0;
	chasing_flash_cycle_counter = cycle_counter_for_chasing_flash_per_interval[counter_for_current_cycle];
}


static void 
cycle_reached_chasing_flash(u16 cycle) {
	u8 i;
	if(cycle >= cycle_params[current_interval])
	{
		if(counter_for_current_cycle == 0)
		{
			for(i = 0; i < 3; i++)
			{
				if(fade_in_out_flag[i]) fade_in_out_flag[i] = 0;
				else fade_in_out_flag[i] = 1;
			}
			if(current_interval == 3)
			{
				cycle_output[0] = ON;
				cycle_output[1] = ON;
				cycle_output[2] = OFF;
			}
			if(current_interval == 7)
			{
				cycle_output[0] = OFF;
				cycle_output[1] = ON;
				cycle_output[2] = ON;
			}
			if(current_interval == 11)
			{
				cycle_output[0] = OFF;
				cycle_output[1] = OFF;
				cycle_output[2] = ON;
			}
			if(current_interval == 15)
			{
				cycle_output[0] = ON;
				cycle_output[1] = OFF;
				cycle_output[2] = OFF;
			}
		}
		if(counter_for_current_cycle == 1)
		{
			if(current_interval == 0)
			{
				cycle_output[0] = OFF;
				cycle_output[1] = OFF;
				cycle_output[2] = OFF;
			}
			if(current_interval == 3)
			{
				cycle_output[0] = ON;
				cycle_output[1] = ON;
				cycle_output[2] = ON;
			}
			if(current_interval == 6)
			{
				cycle_output[0] = OFF;
				cycle_output[1] = OFF;
				cycle_output[2] = OFF;
			}
			if(current_interval == 10)
			{
				cycle_output[0] = ON;
				cycle_output[1] = ON;
				cycle_output[2] = ON;
			}
			if(current_interval == 14)
			{
				cycle_output[0] = OFF;
				cycle_output[1] = OFF;
				cycle_output[2] = OFF;
			}

		}
		if(counter_for_current_cycle == 2)
		{
			for(i = 0; i < 3; i++)
			{
				if(fade_in_out_flag[i]) fade_in_out_flag[i] = 0;
				else fade_in_out_flag[i] = 1;
			}
			if(current_interval == 3)
			{
				cycle_output[2] = ON;
				cycle_output[1] = ON;
				cycle_output[0] = OFF;
			}
			if(current_interval == 7)
			{
				cycle_output[2] = OFF;
				cycle_output[1] = ON;
				cycle_output[0] = ON;
			}
			if(current_interval == 11)
			{
				cycle_output[2] = OFF;
				cycle_output[1] = OFF;
				cycle_output[0] = ON;
			}
			if(current_interval == 15)
			{
				cycle_output[2] = ON;
				cycle_output[1] = OFF;
				cycle_output[0] = OFF;
			}
		}
		if(counter_for_current_cycle == 3)
		{
			if(current_interval == 0)
			{
				cycle_output[0] = OFF;
				cycle_output[1] = OFF;
				cycle_output[2] = OFF;
			}
			if(current_interval == 4)
			{
				cycle_output[0] = ON;
				cycle_output[1] = ON;
				cycle_output[2] = ON;
			}
			if(current_interval == 8)
			{
				cycle_output[0] = OFF;
				cycle_output[1] = OFF;
				cycle_output[2] = OFF;
			}
			if(current_interval == 12)
			{
				cycle_output[0] = ON;
				cycle_output[1] = ON;
				cycle_output[2] = ON;
			}
			if(current_interval == 14)
			{
				cycle_output[0] = OFF;
				cycle_output[1] = OFF;
				cycle_output[2] = OFF;
			}
		}
		current_interval ++;
		if(current_interval == 16)
		{
			current_interval = 0;
			-- chasing_flash_cycle_counter;
			if(chasing_flash_cycle_counter == 0)
			{
				counter_for_current_cycle++;
				if(counter_for_current_cycle == 4)
				{
					counter_for_current_cycle = 0;
				}
				if(counter_for_current_cycle == 0)
				{
					cycle_output[0] = ON;
					cycle_output[1] = OFF;
					cycle_output[2] = OFF;
				}
				if(counter_for_current_cycle == 2)
				{
					cycle_output[2] = ON;
					cycle_output[1] = OFF;
					cycle_output[0] = OFF;
				}
				cycle_params = cycle_for_chasing_flash[counter_for_current_cycle]; 
				chasing_flash_cycle_counter = cycle_counter_for_chasing_flash_per_interval[counter_for_current_cycle];
			}
		}
		reset_counter_for_cycle();
	}
}
// 5
// slow_fade
static void 
slow_fade_driver(void) {
	u8 i;
	u16 current_counter = counter_for_cycle;
	for(i = 0; i < 3; i++)
	{
		u16 progress = (current_counter * 100UL) / (cycle_for_slow_fade[current_interval]) ;
		u16 reversed_cycle_rate_of_progress = 100 - progress;

		if (fade_in_out_flag[i] == RISE) {
		// 0 -> 10
			lumins_level_logic_prepared[i] = progress / fade_in_out_fine_grained_level;
			lumins_level_deviation[i] = progress - lumins_level_logic_prepared[i] * fade_in_out_fine_grained_level;
		} else { // FALL
		// 10 -> 0
			lumins_level_logic_prepared[i] = reversed_cycle_rate_of_progress / fade_in_out_fine_grained_level;
			lumins_level_deviation[i] = reversed_cycle_rate_of_progress - lumins_level_logic_prepared[i] * fade_in_out_fine_grained_level;
		}
	}
}

static void
slow_fade_driver_init() {
	cycle_output[0] = ON;
	cycle_output[1] = ON;
	cycle_output[2] = ON;
	fade_in_out_flag[0] = RISE;
	fade_in_out_flag[1] = RISE;
	fade_in_out_flag[2] = RISE;	
	lumins_level_logic_prepared[0] = lumins_level_min;
	lumins_level_logic_prepared[1] = lumins_level_min;
	lumins_level_logic_prepared[2] = lumins_level_min;
	current_interval = 0;
}


static void 
cycle_reached_slow_fade(u16 cycle) {
	u8 i;
	if(cycle >= cycle_params[current_interval])
	{
		for(i = 0; i < 3; i++)
		{
			if(fade_in_out_flag[i]) fade_in_out_flag[i] = 0;
			else fade_in_out_flag[i] = 1;
		}
		current_interval ++;
		if(current_interval == 16)
		{
			current_interval = 0;
		}
		reset_counter_for_cycle();
	}
}


// 6
// twinkle_flash
static void 
twinkle_flash_driver(void) {
	u8 i;
	u16 current_counter = counter_for_cycle;
	for(i = 0; i < 3; i++)
	{
		u16 progress = (current_counter * 100UL) / (cycle_for_twinkle_flash[current_interval]) ;
		u16 reversed_cycle_rate_of_progress = 100 - progress;

		if (fade_in_out_flag[i] == RISE) {
		// 0 -> 10
			lumins_level_logic_prepared[i] = progress / fade_in_out_fine_grained_level;
			lumins_level_deviation[i] = progress - lumins_level_logic_prepared[i] * fade_in_out_fine_grained_level;
		} else { // FALL
		// 10 -> 0
			lumins_level_logic_prepared[i] = reversed_cycle_rate_of_progress / fade_in_out_fine_grained_level;
			lumins_level_deviation[i] = reversed_cycle_rate_of_progress - lumins_level_logic_prepared[i] * fade_in_out_fine_grained_level;
		}
	}
}

static void
twinkle_flash_driver_init() {
	cycle_output[0] = ON;
	cycle_output[1] = OFF;
	cycle_output[2] = OFF;
	fade_in_out_flag[0] = RISE;
	fade_in_out_flag[1] = RISE;
	fade_in_out_flag[2] = RISE;	
	lumins_level_logic_prepared[0] = lumins_level_min;
	lumins_level_logic_prepared[1] = lumins_level_min;
	lumins_level_logic_prepared[2] = lumins_level_min;
	current_interval = 0;
}


static void 
cycle_reached_twinkle_flash(u16 cycle) {
	u8 i;
	if(cycle >= cycle_params[current_interval])
	{
		for(i = 0; i < 3; i++)
		{
			if(fade_in_out_flag[i]) fade_in_out_flag[i] = 0;
			else fade_in_out_flag[i] = 1;
		}
		if(current_interval == 3)
		{
			cycle_output[0] = ON;
			cycle_output[1] = ON;
			cycle_output[2] = OFF;
		}
		if(current_interval == 7)
		{
			cycle_output[0] = OFF;
			cycle_output[1] = ON;
			cycle_output[2] = ON;
		}
		if(current_interval == 11)
		{
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = ON;
		}
		if(current_interval == 15)
		{
			cycle_output[0] = ON;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		current_interval ++;
		if(current_interval == 16)
		{
			current_interval = 0;
		}
		reset_counter_for_cycle();
	}
}

//7
//stacking_flash
static void 
stacking_flash_driver(void) {
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}

static void
stacking_flash_driver_init() {
	cycle_output[0] = ON;
	cycle_output[1] = OFF;
	cycle_output[2] = OFF;
	current_interval = 0;
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}


static void 
cycle_reached_stacking_flash(u16 cycle) {

	if(cycle >= cycle_params[speed_level])
	{
		if(current_interval == 0){
//			cycle_output[0] = OFF;
			cycle_output[1] = ON;
//			cycle_output[2] = ON;
		}
		else if(current_interval == 1){
//			cycle_output[0] = ON;
//			cycle_output[1] = ON;
			cycle_output[2] = ON;
		}
		else if(current_interval == 2){
//			cycle_output[0] = OFF;
//			cycle_output[1] = ON;
//			cycle_output[2] = ON;
		}
		else if(current_interval == 3){
//			   cycle_output[0] = OFF;
			// cycle_output[1] = ON;
			 cycle_output[2] = OFF;
		}
		else if(current_interval == 4){
//			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			// cycle_output[2] = ON;
		}
		else if(current_interval == 5){
			cycle_output[0] = OFF;
//			cycle_output[1] = OFF;
			//cycle_output[2] = OFF;
		}
		else if(current_interval == 6)
		{
			cycle_output[0] = ON;
//			cycle_output[1] = OFF;
			//cycle_output[2] = OFF;
		}
// 		else if(current_interval == 11){
// 			cycle_output[0] = ON;
// //			cycle_output[1] = OFF;
// //			cycle_output[2] = OFF;
// 		}
// 		// 6 -> 10 继续关闭全部
// 		else{ 
// //			cycle_output[0] = OFF;
// //			cycle_output[1] = OFF;
// //			cycle_output[2] = OFF;
// 		}
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 7)
			current_interval = 0;
	}
}

// 8
// flash_and_chase
static void 
flash_and_chase_driver(void) {
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}

static void
flash_and_chase_driver_init() {
	cycle_output[0] = OFF;
	cycle_output[1] = OFF;
	cycle_output[2] = OFF;
	current_interval = 0;
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}


static void 
cycle_reached_flash_and_chase(u16 cycle) {

	if(cycle >= cycle_params[speed_level])
	{
		if(current_interval == 0){
			cycle_output[0] = ON;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 1){
			cycle_output[0] = ON;
			cycle_output[1] = ON;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 2){
			cycle_output[0] = ON;
			cycle_output[1] = ON;
			cycle_output[2] = ON;
		}
		else if(current_interval == 3){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 4)
			current_interval = 0;
	}
}

// 9
// random_flashing
static void 
random_flashing_driver(void) {
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}

static void
random_flashing_driver_init() {
	cycle_output[0] = OFF;
	cycle_output[1] = OFF;
	cycle_output[2] = OFF;
	current_interval = 0;
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}


static void 
cycle_reached_random_flashing(u16 cycle) {

	if(cycle >= cycle_params[speed_level])
	{
		if(current_interval == 0){
			cycle_output[0] = OFF;
			cycle_output[1] = ON;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 1){
			cycle_output[0] = ON;
			cycle_output[1] = ON;
			cycle_output[2] = ON;
		}
		else if(current_interval == 2){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 3)
			current_interval = 0;
	}
}

// 10
// pulsing
static void 
pulsing_driver(void) {
	u8 i;
	for(i = 0; i < 3; i++)
		lumins_level_logic_prepared[i] = (dim_in_out_flag == HALF) ? lumins_level_set >> 1 : lumins_level_set;
}

static void
pulsing_driver_init() {
	cycle_output[0] = ON;
	cycle_output[1] = ON;
	cycle_output[2] = ON;
	dim_in_out_flag = HALF;
	current_interval = 0;
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}

static void 
cycle_reached_pulsing(u16 cycle) {
	if(cycle >= cycle_params[speed_level]){
		reverse_dim_in_out_flag();
		reset_counter_for_cycle();
	}
}

// 11
// sequential_flashing
static void 
sequential_flashing_driver()
{
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}

static void
sequential_flashing_driver_init() {
	cycle_output[0] = OFF;
	cycle_output[1] = OFF;
	cycle_output[2] = OFF;
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
	current_interval = 0;
}

static void 
cycle_reached_sequential_flashing(u16 cycle)
{
	if(cycle >= cycle_params[speed_level])
	{
		if(current_interval == 0){
			cycle_output[0] = ON;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 1){
			cycle_output[0] = OFF;
			cycle_output[1] = ON;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 2){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = ON;
		}

		else if(current_interval == 3){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = ON;
		}
		else if(current_interval == 4){
			cycle_output[0] = OFF;
			cycle_output[1] = ON;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 5){
			cycle_output[0] = ON;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 6){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 7)
			current_interval = 0;
	}
}

// 12
// bi_directional_storbing
static void 
bi_directional_storbing_driver(void) {
	if(current_interval == 0)
	{
		lumins_level_logic_prepared[0] = lumins_level_set;
		lumins_level_logic_prepared[1] = lumins_level_set;
		lumins_level_logic_prepared[2] = lumins_level_set;
	}
	if(current_interval == 8)
	{
		lumins_level_logic_prepared[0] = lumins_level_set >> 1;
		lumins_level_logic_prepared[1] = lumins_level_set >> 1;
		lumins_level_logic_prepared[2] = lumins_level_set >> 1;
	}
	if(current_interval == 16)
	{
		lumins_level_logic_prepared[0] = lumins_level_set >> 2;
		lumins_level_logic_prepared[1] = lumins_level_set >> 2;
		lumins_level_logic_prepared[2] = lumins_level_set >> 2;
	}
}

static void
bi_directional_storbing_driver_init() {
	cycle_output[0] = ON;
	cycle_output[1] = ON;
	cycle_output[2] = ON;
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
	current_interval = 0;
}

static void 
cycle_reached_bi_directional_storbing(u16 cycle) {
	u8 i;
	if(cycle >= cycle_params[speed_level])
	{
		for(i = 0; i < 3; i++)
		{
			if(cycle_output[i]) cycle_output[i] = 0;
			else cycle_output[i] = 1;
		}
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 25)
			current_interval = 0;

	}
}

// 13
// steady_on
static void 
steady_on_driver(void) {
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}

static void
steady_on_driver_init() {
	cycle_output[0] = ON;
	cycle_output[1] = ON;
	cycle_output[2] = ON;
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}

// 14
/*
*	Sensor Mode
*/

static void 
sensor_driver() {
	u8 lumins_level_logic_prepared_for_deviation[3];
	// if (sensor_should_reinit_from_awake()) {
	// 	sensor_driver_init();
	// } else {
	if(!--sensor_counter)
	{
		int level_to_average;
	    u8 level_below_avg_threshold;
	    u16 current_adc_max;
	    sensor_counter = 4;

		// 6 * 60Hz period max
	    period_max_buffer[period_max_index] = read_adc_period_max();
	    refresh_adc_period_max();
	    ++period_max_index;
	    // 保留上一次亮度值
		lumins_level_logic_prepared_for_deviation[0] = lumins_level_logic_prepared[0];
		lumins_level_logic_prepared_for_deviation[1] = lumins_level_logic_prepared[1];
		lumins_level_logic_prepared_for_deviation[2] = lumins_level_logic_prepared[2];

	    if (period_max_index == PERIOD_MAX_SIZE) 
	    {
	    	// -------------------------------- sampling ----------------------------------
	    	period_max_index = 0;

		    current_adc_max = find_period_max_in_buffer();

		    max_avg = max_sum >> AVG_BUFFER_SIZE_BIT;

			level_to_average = current_adc_max - max_avg;
			max_sum -= avg_buffer[max_avg_index];
			max_sum += current_adc_max;
			avg_buffer[max_avg_index] = current_adc_max;
			spiral_increase_q(max_avg_index, AVG_BUFFER_SIZE);
			if (sensor_initializing_finished())
				sensor_initializing = NO;
			// -------------------------------- choosing ----------------------------------
			// shift
			if (sensor_initializing)
			{
				lumins_level_logic_prepared[0] = lumins_level_min;
				lumins_level_logic_prepared[1] = lumins_level_min;
				lumins_level_logic_prepared[2] = lumins_level_min;
			}
			else 
			{
				max_avg = max_avg >> 2;
				if (belong_to_interval_lcro(max_avg, avg_range[0], avg_range[1])) {
					diff = diff_avg_range_1;
					level_below_avg_threshold = (below_avg_threshold[0]); 
				}
				else if (belong_to_interval_lcro(max_avg, avg_range[1], avg_range[2])) {
					diff = diff_avg_range_2;
					level_below_avg_threshold = (below_avg_threshold[1]); 
				}
				else if (belong_to_interval_lcro(max_avg, avg_range[2], avg_range[3])) {
					diff = diff_avg_range_3;
					level_below_avg_threshold = (below_avg_threshold[2]); 
				}
				else {
					diff = diff_avg_range_4;
					level_below_avg_threshold = (below_avg_threshold[3]); 
				}
				if (level_to_average < 0) 
				{
					level_to_average = -level_to_average;
					if (belong_to_interval_lc(level_to_average, level_below_avg_threshold))
					{
					    lumins_level_logic_prepared[0] = lumins_level_min;
					    lumins_level_logic_prepared[1] = lumins_level_min;
					    lumins_level_logic_prepared[2] = lumins_level_min;
					}
					else if (belong_to_interval_lcro(level_to_average, below_avg_threshhold_for_peace, level_below_avg_threshold))
					{
					    lumins_level_logic_prepared[0] = 1;
					    lumins_level_logic_prepared[1] = 1;
					    lumins_level_logic_prepared[2] = 1;
					}
					else	// 0 - below_avg_threshhold_for_peace
					{
						lumins_level_logic_prepared[0] = lumins_level_peace;
						lumins_level_logic_prepared[1] = lumins_level_peace;
						lumins_level_logic_prepared[2] = lumins_level_peace;
					}

				} 
				else 
				{ 
					//level_to_average = level_to_average >> 2;
					if (belong_to_interval_lcro(level_to_average, 0, diff[0])) {
					    lumins_level_logic_prepared[0] = lumins_level_peace;
					    lumins_level_logic_prepared[1] = lumins_level_peace;
					    lumins_level_logic_prepared[2] = lumins_level_peace;
					} else if (belong_to_interval_lcro(level_to_average, diff[0], diff[1])) {
					    lumins_level_logic_prepared[0] = 3;
					    lumins_level_logic_prepared[1] = 3;
					    lumins_level_logic_prepared[2] = 3;
					} else if (belong_to_interval_lcro(level_to_average, diff[1], diff[2])) {
					    lumins_level_logic_prepared[0] = 4;
					    lumins_level_logic_prepared[1] = 4;
					    lumins_level_logic_prepared[2] = 4;
					} else if (belong_to_interval_lcro(level_to_average, diff[2], diff[3])) {
					    lumins_level_logic_prepared[0] = 5;
					    lumins_level_logic_prepared[1] = 5;
					    lumins_level_logic_prepared[2] = 5;
					} else if (belong_to_interval_lcro(level_to_average, diff[3], diff[4])) {
					    lumins_level_logic_prepared[0] = 6;
					    lumins_level_logic_prepared[1] = 6;
					    lumins_level_logic_prepared[2] = 6;
					} else if (belong_to_interval_lcro(level_to_average, diff[4], diff[5])) {
					    lumins_level_logic_prepared[0] = 7;
					    lumins_level_logic_prepared[1] = 7;
					    lumins_level_logic_prepared[2] = 7;
					} else if (belong_to_interval_lcro(level_to_average, diff[5], diff[6])) {
					    lumins_level_logic_prepared[0] = 8;
					    lumins_level_logic_prepared[1] = 8;
					    lumins_level_logic_prepared[2] = 8;
					} else if (belong_to_interval_lcro(level_to_average, diff[6], diff[7])) {
					    lumins_level_logic_prepared[0] = 9;
					    lumins_level_logic_prepared[1] = 9;
					    lumins_level_logic_prepared[2] = 9;
					} else if (belong_to_interval_lc(level_to_average, diff[7])) {
					    lumins_level_logic_prepared[0] = lumins_level_max;
					    lumins_level_logic_prepared[1] = lumins_level_max;
					    lumins_level_logic_prepared[2] = lumins_level_max;
					}
				}
			}
			// }
		} // end-of-first-if
	// }
		if(display_mode_logic == in_wave)
		{
			lumins_level_logic_prepared[1] = lumins_level_logic_prepared_for_deviation[0];
			lumins_level_logic_prepared[2] = lumins_level_logic_prepared_for_deviation[1];
		}
		if(display_mode_logic == slow_fade)
		{
			u8 i;
			for(i = 0; i < 3; i++){
				if(!lumins_level_logic_prepared_for_deviation[i])
					lumins_level_logic_prepared[i] = lumins_level_logic_prepared_for_deviation[i] - 1;		
			}
		}
		if(display_mode_logic == pulsing)
		{
			u8 i;
			for(i = 0; i < 3; i++)
				lumins_level_logic_prepared[i] = (dim_in_out_flag == HALF) ? lumins_level_logic_prepared[i] >> 1 : ((lumins_level_logic_prepared[i] >> 1) + 5);
		}
		
		if(display_mode_logic == bi_directional_storbing)
		{
			if(current_interval == 0)
			{
				lumins_level_logic_prepared[0] = lumins_level_logic_prepared[0];
				lumins_level_logic_prepared[1] = lumins_level_logic_prepared[1];
				lumins_level_logic_prepared[2] = lumins_level_logic_prepared[2];
			}
			if(current_interval == 8)
			{
				lumins_level_logic_prepared[0] = lumins_level_logic_prepared[0] >> 1;
				lumins_level_logic_prepared[1] = lumins_level_logic_prepared[1] >> 1;
				lumins_level_logic_prepared[2] = lumins_level_logic_prepared[2] >> 1;
			}
			if(current_interval == 16)
			{
				lumins_level_logic_prepared[0] = lumins_level_logic_prepared[0] >> 2;
				lumins_level_logic_prepared[1] = lumins_level_logic_prepared[1] >> 2;
				lumins_level_logic_prepared[2] = lumins_level_logic_prepared[2] >> 2;
			}
		}
	}
}

static void
sensor_driver_init() {
	
	u8 buffer_size = AVG_BUFFER_SIZE;
	u8 i;
	sensor_initializing = YES;
	current_interval = 0;
	// quick output params response
	// if(!P30)
	// {
	// 	cycle_output[0] = ON;
	// 	cycle_output[1] = ON;
	// 	cycle_output[2] = ON;
	// }
	// else
	// {
	// 	cycle_output[0] = ON;
	// 	cycle_output[1] = OFF;
	// 	cycle_output[2] = OFF;
	// } 
	// lumins_level_logic_prepared[0] = lumins_level_peace;
	// lumins_level_logic_prepared[1] = lumins_level_peace;
	// lumins_level_logic_prepared[2] = lumins_level_peace;
	max_avg_index = 0;
	// Reset buffer
	for (i = buffer_size; i; )
		avg_buffer[--i] = BASE_AVG;
	max_sum = BASE_AVG << AVG_BUFFER_SIZE_BIT;
	max_avg = BASE_AVG;
	period_max_index = 0;
	buffer_size = PERIOD_MAX_SIZE;
	for (i = buffer_size; i; )
		period_max_buffer[--i] = 0;
	// adc_enable();
}


