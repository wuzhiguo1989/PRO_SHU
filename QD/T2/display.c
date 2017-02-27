#include "shared.h"
#include "display.h"
#include "sync.h"
#include "def.h"
#include "intrins.h"
#include "adc.h"
#include "uart.h"
#include "nRF24L01.h"
// ------------------------- display config declaring -------------------------
static code disp_t const
// 重新组织灯光模式
next_display_mode[14] = {flash2_directional, random_fade, stacking_flash, flash_and_chase, random_flashing, bi_directional_pulsing, bi_directional_chasing, pulsing, steady_flash, sequential_flashing, progressive_strobing, bi_directional_storbing, fade_in_fade_out,flash1_directional};
static u8 
next_combination_display_mode_counter[14] = {6, 6, 1, 4, 3, 2, 2, 3, 3, 6, 3, 1, 1, 1};

static code disp_t const
next_sensor_display_mode[15] = {steady_flash, flash1_directional, sequential_flashing, bi_directional_chasing, bi_directional_storbing, random_flashing,  flash_and_chase,
	stacking_flash, bi_directional_chasing, flash2_directional, flash_and_chase, flash1_directional, flash2_directional, flash1_directional, sequential_flashing};
static u8 
next_sensor_display_mode_counter[15] = {7, 1, 2, 2, 1, 6, 2, 1, 1, 1, 2, 1, 5, 1, 3};

// combination start from second disp_t
#define next_display_mode_from 	1

// TODO 60Hz is ok, 50Hz is not
static code u16 const
//phase_delay_for_60Hz[10] = {0xD99A, 0xDACD, 0xDD80, 0xDFE6, 0xE34D, 0xE4DA, 0xE740, 0xE9CD, 0xEC33, 0xF100};
phase_delay_for_60Hz[10] = {0x2666, 0x2533, 0x2280, 0x201A, 0x1CB3, 0x1B26, 0x18C0, 0x1633, 0x13CD, 0x0F00};
// Cycle are calculated based on 5ms PCA timer -> sync.h
// 1 + 2
static code u16 const 
cycle_for_flash_directional[11]= {67, 60, 53, 46, 39, 33, 28, 22, 16, 11, 5};
// 3
static code u16 const 
cycle_for_random_fade[11] = {916, 833, 750, 666, 583, 500, 416, 333, 250, 166, 83};
// 4
static code u16 const 
cycle_for_stacking_flash[11] = {67, 60, 53, 46, 39, 33, 28, 22, 16, 11, 5};
// 5
static code u16 const 
cycle_for_flash_and_chase[11] = {67, 60, 53, 46, 39, 33, 28, 22, 16, 11, 5};
// 6
static code u16 const 
cycle_for_random_flashing[11][5] = {
	{67, 25, 135, 67, 36},
	{60, 22, 120, 60, 33},
	{53, 20, 106, 53, 30},
	{46, 18, 92, 46, 27},
	{39, 16, 78, 39, 23},
	{33, 13, 67, 33, 20},
	{28, 10, 56, 28, 16},
	{22, 8, 44, 22, 13},
	{16, 6, 32, 16, 10},
	{11, 4, 22, 11, 7},
	{5, 2, 10, 5, 3}
};
// 7
// TODO
static code u16 const 
cycle_for_bi_directional_pulsing[11][6] = {
	{305, 611, 916, 1221, 1527, 1832},
	{278, 555, 833, 1111, 1389, 1667},
	{250, 500, 750, 1000, 1250, 1500},
	{222, 444, 666, 888, 1110, 1332},
	{195, 390, 583, 777, 971, 1166},
	{167, 333, 500, 667, 833, 1000},
	{139, 278, 416, 556, 695, 832},
	{111, 222, 333, 444, 555, 666},
	{83, 166, 250, 333, 416, 500},
	{55, 110, 166, 221, 276, 331},
	{28, 56, 83, 110, 138, 165}
};
// 8
static code u16 const 
cycle_for_bi_directional_chasing[11] = {67, 60, 53, 46, 39, 33, 28, 22, 16, 11, 5};

// 9
static code u16 const 
cycle_for_pulsing[11] = {367, 333, 300, 267, 233, 200, 167, 133, 100, 67, 33};
// 10
static code u16 const 
cycle_for_steady_flash[11] = {275, 250, 200, 175, 150, 125, 100, 75, 50, 25};
// 11
static code u16 const 
cycle_for_sequential_flashing[11] = {67, 60, 53, 46, 39, 33, 28, 22, 16, 11, 5};
// 12
static code u16 const 
cycle_for_progressive_strobing[11][9] ={
	{550, 390, 367, 293, 220, 132, 84, 59, 29},
	{500, 360, 333, 267, 200, 120, 76, 53, 27},
	{450, 330, 300, 240, 180, 108, 68, 48, 24},
	{400, 300, 267, 213, 160, 96, 62, 42, 22},
	{350, 270, 233, 187, 140, 84, 56, 37, 19},
	{300, 240, 200, 160, 120, 72, 48, 32, 16},
	{250, 180, 167, 133, 100, 60, 40, 27, 13},
	{200, 150, 133, 107, 80, 48, 32, 22, 10},
	{150, 120, 100, 80, 60, 36, 24, 16, 8},
	{100, 80, 67, 53, 40, 24, 16, 11, 5},
	{50, 40, 33, 27, 20, 12, 8, 5, 3}
};
// 13
static code u16 const
cycle_for_bi_directional_storbing[11][8] ={
	{51, 51, 51, 51, 37, 51, 51, 51},
	{47, 47, 47, 47, 33, 47, 47, 47},
	{42, 42, 42, 42, 30, 42, 42, 42},
	{37, 37, 37, 37, 27, 37, 37, 37},
	{33, 33, 33, 33, 24, 33, 33, 33},
	{28, 28, 28, 28, 20, 28, 28, 28},
	{23, 23, 23, 23, 16, 23, 23, 23},
	{19, 19, 19, 19, 13, 19, 19, 19,},
	{14, 14, 14, 14, 10, 14, 14, 14},
	{9, 9, 9, 9, 7, 9, 9, 9},
	{5, 5, 5, 5, 4, 5, 5, 5}
};
// 14
static code u16 const 
cycle_for_fade_in_fade_out[11] = {916, 833, 750, 666, 583, 500, 416, 333, 250, 166, 83};

static code u16 const 
cycle_for_sensor[7] = {2, 2, 2, 2, 2, 2, 2};

#define progress_strobing_interval_level 9

//static code u16   cycle_for_not_effective_default[11] = {0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff};


#define cycle_for_not_effective_default NULL

static code u8 const 
// strobing_half_cycle_counter_per_interval[9] = {4, 4, 4, 6, 6, 8, 8, 10, 12};
strobing_half_cycle_counter_per_interval[9] = {8, 8, 8, 12, 12, 16, 16, 20, 24};

static code u8 const 
bi_directional_storbing_half_cycle_counter_per_interval[8] = {14, 14, 14, 14, 10, 14, 14, 14};
// bi_directional_storbing_half_cycle_counter_per_interval[8] = {7, 7, 7, 7, 5, 7, 7, 7};

#define BASE_AVG 0x7A
// Sensor mode calculation based
// Lumins level 3 - 10
static code u8 const 
diff_avg_range_1[8] = {3, 4, 5, 6, 7, 8, 9, 10};
static code u8 const   
diff_avg_range_2[8] = {3, 5, 7, 7, 12, 15, 19, 23};
static code u8 const   
diff_avg_range_3[8] = {4, 7, 11, 15, 20, 26, 31, 35};
static code u8 const   
diff_avg_range_4[8] = {4, 7, 11, 16, 22, 28, 33, 41};

// Lumins level 2 for max_of_period_max < max_avg
#define below_avg_threshhold_for_peace		8
// Lumins level 1 for each range
static code u8 const   
below_avg_threshold[4] = {10, 11, 12, 13};
static code u8 const   
avg_range[4] = {0x00, 0x23, 0x26, 0x2f};

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
static u8 finish_cycle_counter = 0;
// sensor_logic 下标计数
static u8 sersor_current_logic = 0;

// static u8 current_interval = 0;

#define fade_in_out_fine_grained_level 10
//static bit fade_in_out_flag = RISE;
static u8 fade_in_out_flag[3] = {RISE, RISE, RISE};
static bit dim_in_out_flag = FULL;
#define lumins_level_min_deviation_interval_for_60Hz  120
static u8 current_interval = 0;
static u8 bi_directional_storbing_counter;
//static u8 current_strobing_interval = 0;
// -> strobing_half_cycle_counter_per_interval
static u8 strobing_half_cycle_counter;

static const u8 code * data diff;

#define AVG_BUFFER_SIZE_BIT 6
#define AVG_BUFFER_SIZE 	(1 << AVG_BUFFER_SIZE_BIT)
#define PERIOD_MAX_SIZE 	4
static xdata u16 avg_buffer[AVG_BUFFER_SIZE];
static u16 period_max_buffer[PERIOD_MAX_SIZE];
static u16 current_adc_max, max_sum = BASE_AVG << AVG_BUFFER_SIZE_BIT, max_avg = BASE_AVG;
static u8 max_avg_index = 0, period_max_index = 0;
static toggle_t toggle_stat_last_for_sensor_prevent_inst_blink = ON;
static BOOL sensor_initializing = NO;

// ------------------------- marco functions -------------------------

#define reverse_dim_in_out_flag() dim_in_out_flag = ~dim_in_out_flag 
#define sync_up_toggle_stat()	toggle_stat_last_for_sensor_prevent_inst_blink = toggle_status
#define index_of(x) x - 1
//#define set_phase_timer(x) TL0 = x; TH0 = x >> 8
#define lumins_level_fine_override_should_change(target) lumins_level_fine_override = (target == fade_in_fade_out || target == random_fade || target == bi_directional_pulsing) ? YES : NO
#define sensor_should_reinit_from_awake()	( toggle_status == ON && toggle_stat_last_for_sensor_prevent_inst_blink == OFF)
#define sensor_initializing_finished()	(max_avg_index == (AVG_BUFFER_SIZE >> 2))
// ------------------------- private declaring -------------------------
//static void half_cycle_reached();
static u16 recalibrate_phase_delay(  lumins_level_t lumins,   u8 deviation);
static u16 find_period_max_in_buffer();
static void sync_with_target_satellite();

// ------------------------- display_mode_init declaring -------------------------
static void combination_driver_init();
static void flash1_directional_driver_init();
static void flash2_directional_driver_init();
static void random_fade_driver_init();
static void stacking_flash_driver_init();
static void flash_and_chase_driver_init();
static void random_flashing_driver_init();
static void bi_directional_pulsing_driver_init();
static void bi_directional_chasing_driver_init();
static void pulsing_driver_init();
static void steady_flash_driver_init();
static void sequential_flashing_driver_init();
static void progressive_strobing_driver_init();
static void bi_directional_storbing_driver_init();
static void fade_in_fade_out_driver_init();
static void steady_burn_driver_init();
static void sensor_driver_init();



// ------------------------- display_mode_driver declaring -------------------------
//static void combination_driver();
static void flash1_directional_driver();
static void flash2_directional_driver();
static void random_fade_driver();
static void stacking_flash_driver();
static void flash_and_chase_driver();
static void random_flashing_driver();
static void bi_directional_pulsing_driver();
static void bi_directional_chasing_driver();
static void pulsing_driver();
static void steady_flash_driver();
static void sequential_flashing_driver();
static void progressive_strobing_driver();
static void bi_directional_storbing_driver();
static void fade_in_fade_out_driver();
static void steady_burn_driver();
static void sensor_driver();

// ------------------------- display_mode_cycle_reached declaring -------------------------
static void cycle_reached_flash1_directional(u16);
static void cycle_reached_flash2_directional(u16);
static void cycle_reached_random_fade(u16);
static void cycle_reached_stacking_flash(u16);
static void cycle_reached_flash_and_chase(u16);
static void cycle_reached_random_flashing(u16);
static void cycle_reached_bi_directional_pulsing(u16);
static void cycle_reached_bi_directional_chasing(u16);
static void cycle_reached_pulsing(u16);
static void cycle_reached_steady_flash(u16);
static void cycle_reached_sequential_flashing(u16);
static void cycle_reached_progressive_strobing(u16);
static void cycle_reached_bi_directional_storbing(u16);
static void cycle_reached_fade_in_fade_out(u16);
//static void cycle_reached_steady_burn();
static void cycle_reached_sensor(u16);


// ------------------------- public -------------------------
// ONLY CALL BY main_isr
// CAUTIONS!  DON't CALL THIS
void display_startup_init() {
	combination_driver_init();									// 1. no need to call steady_driver_init
	display_driver = steady_burn_driver;	// 2. dispatch display_driver
	cycle_params = cycle_for_not_effective_default; // 3. dispatch cycle_params
	_nop_();											// 4. lumins_level_fine_override set by default to NO
	_nop_();											// 5. counters are init by sync.c
	current_phase_delay = phase_delay_for_60Hz;			// 6. reset current phase_delay
}

void
prepare_next_display_params() {
	display_driver();
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

	if (!lumins_level_fine_override) {
		// Rx2Buf0[0] = cycle_output[0];
		Tx2Buf0[0] = calibrated_phase_delay[0] >> 8;
		Tx2Buf0[1] = calibrated_phase_delay[0];
		// Rx2Buf1[0] = cycle_output[1];
		Tx2Buf1[0] = calibrated_phase_delay[1] >> 8;
		Tx2Buf1[1] = calibrated_phase_delay[1];
		// Rx2Buf2[0] = cycle_output[2];
		Tx2Buf2[0] = calibrated_phase_delay[2] >> 8;
		Tx2Buf2[1] = calibrated_phase_delay[2];
		
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

		// Rx2Buf0[0] = cycle_output[0];
		Tx2Buf0[0] = calibrated_phase_delay[0] >> 8;
		Tx2Buf0[1] = calibrated_phase_delay[0];
		// Rx2Buf1[0] = cycle_output[1];
		Tx2Buf1[0] = calibrated_phase_delay[1] >> 8;
		Tx2Buf1[1] = calibrated_phase_delay[1];
		// Rx2Buf2[0] = cycle_output[2];
		Tx2Buf2[0] = calibrated_phase_delay[2] >> 8;
		Tx2Buf2[1] = calibrated_phase_delay[2];

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
	sync_with_target_satellite();
}

void 
cycle_based_adjust(const u16 cycle) {
	if(display_mode_logic == steady_burn)
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
		case flash1_directional: 
			flash1_directional_driver_init();
			display_driver = flash1_directional_driver;
			cycle_params = cycle_for_flash_directional;
			cycle_reached = cycle_reached_flash1_directional;
			break;
		case flash2_directional: 
			flash2_directional_driver_init();
			display_driver = flash2_directional_driver;
			cycle_params = cycle_for_flash_directional;
			cycle_reached = cycle_reached_flash2_directional;
			break;
		case random_fade: 
			random_fade_driver_init();
			display_driver = random_fade_driver;
			cycle_params = cycle_for_random_fade;
			cycle_reached = cycle_reached_random_fade;
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
			cycle_params = cycle_for_random_flashing[speed_level];
			cycle_reached = cycle_reached_random_flashing;
			break;
		case bi_directional_pulsing: 
			bi_directional_pulsing_driver_init();
			display_driver = bi_directional_pulsing_driver;
			cycle_params = cycle_for_bi_directional_pulsing[speed_level];
			cycle_reached = cycle_reached_bi_directional_pulsing;
			break;
		case bi_directional_chasing: 
			bi_directional_chasing_driver_init();
			display_driver = bi_directional_chasing_driver;
			cycle_params = cycle_for_bi_directional_chasing;
			cycle_reached = cycle_reached_bi_directional_chasing;
			break;
		case pulsing: 
			pulsing_driver_init();
			display_driver = pulsing_driver;
			cycle_params = cycle_for_pulsing;
			cycle_reached = cycle_reached_pulsing;
			break;
		case steady_flash: 
			steady_flash_driver_init();
			display_driver = steady_flash_driver;
			cycle_params = cycle_for_steady_flash;
			cycle_reached = cycle_reached_steady_flash;
			break;
		case sequential_flashing: 
			sequential_flashing_driver_init();
			display_driver = sequential_flashing_driver;
			cycle_params = cycle_for_sequential_flashing;
			cycle_reached = cycle_reached_sequential_flashing;
			break;
		case progressive_strobing: 
			progressive_strobing_driver_init();
			display_driver = progressive_strobing_driver;
			// current_strobing_interval set in init()
			cycle_params = cycle_for_progressive_strobing[speed_level];
			cycle_reached = cycle_reached_progressive_strobing;
			break;
		case bi_directional_storbing: 
			bi_directional_storbing_driver_init();
			display_driver = bi_directional_storbing_driver;
			cycle_params = cycle_for_bi_directional_storbing[speed_level];
			cycle_reached = cycle_reached_bi_directional_storbing;
			break;
		case fade_in_fade_out: 
			fade_in_fade_out_driver_init();
			display_driver = fade_in_fade_out_driver;
			cycle_params = cycle_for_fade_in_fade_out;
			cycle_reached = cycle_reached_fade_in_fade_out;
			break;
		case steady_burn: 
			steady_burn_driver_init();
			display_driver = steady_burn_driver;
			cycle_params = cycle_for_not_effective_default;
			break;		
		case sensor:
			sensor_driver_init();
			display_driver = sensor_driver;
			cycle_params = cycle_for_sensor;
			cycle_reached = cycle_reached_sensor;
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
		case flash1_directional: 
			flash1_directional_driver_init();
			display_driver = flash1_directional_driver;
			cycle_params = cycle_for_flash_directional;
			cycle_reached = cycle_reached_flash1_directional;
			break;
		case flash2_directional: 
			flash2_directional_driver_init();
			display_driver = flash2_directional_driver;
			cycle_params = cycle_for_flash_directional;
			cycle_reached = cycle_reached_flash2_directional;
			break;
		case random_fade: 
			random_fade_driver_init();
			display_driver = random_fade_driver;
			cycle_params = cycle_for_random_fade;
			cycle_reached = cycle_reached_random_fade;
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
			cycle_params = cycle_for_random_flashing[speed_level];
			cycle_reached = cycle_reached_random_flashing;
			break;
		case bi_directional_pulsing: 
			bi_directional_pulsing_driver_init();
			display_driver = bi_directional_pulsing_driver;
			cycle_params = cycle_for_bi_directional_pulsing[speed_level];
			cycle_reached = cycle_reached_bi_directional_pulsing;
			break;
		case bi_directional_chasing: 
			bi_directional_chasing_driver_init();
			display_driver = bi_directional_chasing_driver;
			cycle_params = cycle_for_bi_directional_chasing;
			cycle_reached = cycle_reached_bi_directional_chasing;
			break;
		case pulsing: 
			pulsing_driver_init();
			display_driver = pulsing_driver;
			cycle_params = cycle_for_pulsing;
			cycle_reached = cycle_reached_pulsing;
			break;
		case steady_flash: 
			steady_flash_driver_init();
			display_driver = steady_flash_driver;
			cycle_params = cycle_for_steady_flash;
			cycle_reached = cycle_reached_steady_flash;
			break;
		case sequential_flashing: 
			// sequential_flashing_driver_init();
			// display_driver = sequential_flashing_driver;
			// cycle_params = cycle_for_sequential_flashing;
			// cycle_reached = cycle_reached_sequential_flashing;
			break;
		case progressive_strobing: 
			progressive_strobing_driver_init();
			display_driver = progressive_strobing_driver;
			// current_strobing_interval set in init()
			cycle_params = cycle_for_progressive_strobing[speed_level];
			cycle_reached = cycle_reached_progressive_strobing;
			break;
		case bi_directional_storbing: 
			bi_directional_storbing_driver_init();
			display_driver = bi_directional_storbing_driver;
			cycle_params = cycle_for_bi_directional_storbing[speed_level];
			cycle_reached = cycle_reached_bi_directional_storbing;
			break;
		case fade_in_fade_out: 
			fade_in_fade_out_driver_init();
			display_driver = fade_in_fade_out_driver;
			cycle_params = cycle_for_fade_in_fade_out;
			cycle_reached = cycle_reached_fade_in_fade_out;
			break;
		case steady_burn: 
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
	if (display_mode_set == combination) {
		if(finish_cycle_counter == next_combination_display_mode_counter[display_mode_logic - next_display_mode_from])
	 	{
			display_mode_logic = next_display_mode[display_mode_logic - next_display_mode_from];
			finish_cycle_counter = 0;
			display_mode_logic_changed();
		}
	}

}

static void sync_with_target_satellite() {
	INT_CLKO &= 0xEF; // 关闭发送中断
	NRF24L01_TX_Mode();
	//NRF24L01_TxPacket_NUM(Tx2Buf0,0);
	NRF24L01_TxPacket_NUM(Tx2Buf1,1);
	NRF24L01_TxPacket_NUM(Tx2Buf2,2);
	NRF24L01_RX_Mode(4);
	INT_CLKO |= 0x10;
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
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
//	display_mode_logic = slow_flashing;
	display_mode_logic = flash1_directional;
	finish_cycle_counter = 0;
	// cycle_params = cycle_for_flash1_directional;		// 3. dispatch cycle_params
	// cycle_reached = cycle_reached_flash1_directional;
	display_mode_logic_changed();
}

// 1
static void 
flash1_directional_driver(void) {
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}

static void
flash1_directional_driver_init() {
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
	current_interval = 0;
	cycle_output[0] = ON;
	cycle_output[1] = OFF;
	cycle_output[2] = OFF;
}


static void 
cycle_reached_flash1_directional(u16 cycle) 
{
	if(cycle >= cycle_params[speed_level])
	{
		if(current_interval == 0){
			cycle_output[0] = OFF;
			cycle_output[1] = ON;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 1){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = ON;
		}
		else if(current_interval == 2){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 5){
			cycle_output[0] = ON;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		else
		{
		// 3->4 2倍时间做一件事
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 6)
		{
			// 计完成周期
			if(display_mode_set == combination || display_mode_set == sensor)
				finish_cycle_counter ++;
			current_interval = 0;
		}
	}
}

// 2
static void 
flash2_directional_driver(void) {
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}

static void
flash2_directional_driver_init() {
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
	current_interval = 0;
	cycle_output[0] = OFF;
	cycle_output[1] = OFF;
	cycle_output[2] = ON;
}

static void 
cycle_reached_flash2_directional(u16 cycle) 
{
	if(cycle >= cycle_params[speed_level])
	{
		if(current_interval == 0){
			cycle_output[0] = OFF;
			cycle_output[1] = ON;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 1){
			cycle_output[0] = ON;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 2){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 5){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = ON;
		}
		else
		{
		// 3->4 2倍时间做一件事
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 6)
		{
			// 计完成周期
			if(display_mode_set == combination || display_mode_set == sensor)
				finish_cycle_counter ++;
			current_interval = 0;
		}
	}
}
// 3
static void 
random_fade_driver(void) {
	u8 i;
	u16 current_counter = counter_for_cycle;
	for(i = 0; i < 3; i++)
	{
		u16 progress = (current_counter * 100UL) / (cycle_for_fade_in_fade_out[speed_level]) ;
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
random_fade_driver_init() {
	cycle_output[0] = OFF;
	cycle_output[1] = ON;
	cycle_output[2] = ON;
	lumins_level_logic_prepared[0] = lumins_level_min;
	lumins_level_logic_prepared[1] = lumins_level_max;
	lumins_level_logic_prepared[2] = lumins_level_min;
	fade_in_out_flag[0] = RISE;
	fade_in_out_flag[1] = FALL;
	fade_in_out_flag[2] = RISE;
	current_interval = 0;
}


static void 
cycle_reached_random_fade(u16 cycle) {
	if(cycle >= cycle_params[speed_level])
	{
		if(current_interval == 0){
			fade_in_out_flag[0] = FALL;
			fade_in_out_flag[1] = RISE;
			fade_in_out_flag[2] = FALL;
			cycle_output[0] = OFF;
			cycle_output[1] = ON;
			cycle_output[2] = ON;
		}
		else if(current_interval == 1){
			fade_in_out_flag[0] = RISE;
			fade_in_out_flag[1] = FALL;
			fade_in_out_flag[2] = RISE;
			cycle_output[0] = ON;
			cycle_output[1] = ON;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 2){
			fade_in_out_flag[0] = FALL;
			fade_in_out_flag[1] = RISE;
			fade_in_out_flag[2] = FALL;
			cycle_output[0] = ON;
			cycle_output[1] = ON;
			cycle_output[2] = OFF;

		}
		else if(current_interval == 3){
			fade_in_out_flag[0] = RISE;
			fade_in_out_flag[1] = FALL;
			fade_in_out_flag[2] = RISE;
			cycle_output[0] = OFF;
			cycle_output[1] = ON;
			cycle_output[2] = ON;
		}
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 4)
		{
			// 计完成周期
			if(display_mode_set == combination || display_mode_set == sensor)
				finish_cycle_counter ++;
			current_interval = 0;
		}
	}
}



// 4
static void 
stacking_flash_driver(void) {
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}

static void
stacking_flash_driver_init() {
	cycle_output[0] = OFF;
	cycle_output[1] = OFF;
	cycle_output[2] = ON;
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
			cycle_output[0] = ON;
//			cycle_output[1] = ON;
//			cycle_output[2] = ON;
		}
		else if(current_interval == 2){
//			cycle_output[0] = OFF;
//			cycle_output[1] = ON;
//			cycle_output[2] = ON;
		}
		else if(current_interval == 3){
			cycle_output[0] = OFF;
			// cycle_output[1] = ON;
			// cycle_output[2] = ON;
		}
		else if(current_interval == 4){
//			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			// cycle_output[2] = ON;
		}
		else if(current_interval == 5){
//			cycle_output[0] = OFF;
//			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 11){
//			cycle_output[0] = OFF;
//			cycle_output[1] = OFF;
			cycle_output[2] = ON;
		}
		// 6 -> 10 继续关闭全部
		else{ 
//			cycle_output[0] = OFF;
//			cycle_output[1] = OFF;
//			cycle_output[2] = OFF;
		}
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 12)
		{
			// 计完成周期
			if(display_mode_set == combination || display_mode_set == sensor)
				finish_cycle_counter ++;
			current_interval = 0;
		}
	}
}


// 5
static void 
flash_and_chase_driver(void) {
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}

static void
flash_and_chase_driver_init() {
	cycle_output[0] = ON;
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
			cycle_output[0] = OFF;
			cycle_output[1] = ON;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 1){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = ON;
		}
		else if(current_interval == 2){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 3){

			cycle_output[0] = ON;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 4){
			cycle_output[0] = OFF;
			cycle_output[1] = ON;
		 	cycle_output[2] = OFF;
		}
		else if(current_interval == 5){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = ON;
		}
		else if(current_interval == 6){
			cycle_output[0] = ON;
			cycle_output[1] = OFF;
			cycle_output[2] = ON;
		}
		else if(current_interval == 7){
			cycle_output[0] = OFF;
			cycle_output[1] = ON;
			cycle_output[2] = ON;
		}
		else if(current_interval == 8){
			cycle_output[0] = ON;
			cycle_output[1] = ON;
			cycle_output[2] = ON;
		}
		else if(current_interval == 9){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 10){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 11){
			cycle_output[0] = ON;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 12)
		{
			// 计完成周期
			if(display_mode_set == combination || display_mode_set == sensor)
				finish_cycle_counter ++;
			current_interval = 0;
		}
	}
}


// 6
static void 
random_flashing_driver(void) {
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}

static void
random_flashing_driver_init() {
	cycle_output[0] = OFF;
	cycle_output[1] = ON;
	cycle_output[2] = ON;
	current_interval = 0;
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}


static void 
cycle_reached_random_flashing(u16 cycle) {

	if(cycle >= cycle_params[current_interval])
	{
		if(current_interval == 0){
			cycle_output[0] = OFF;
			cycle_output[1] = ON;
			cycle_output[2] = ON;
		}
		else if(current_interval == 1){
			cycle_output[0] = ON;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 2){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 3){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 4){
			cycle_output[0] = OFF;
			cycle_output[1] = ON;
			cycle_output[2] = ON;
		}
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 5)
		{
			// 计完成周期
			if(display_mode_set == combination || display_mode_set == sensor)
				finish_cycle_counter ++;
			current_interval = 0;
		}
	}
}


// 7
static void 
bi_directional_pulsing_driver_init()
{
	cycle_output[0] = ON;
	cycle_output[1] = ON;
	cycle_output[2] = ON;
}
static void 
bi_directional_pulsing_driver()
{
	u8 i,current_progress[3];
	u16 current_counter = counter_for_cycle;
	u16 progress = (current_counter * 200UL) / cycle_params[5];
	if(current_counter <= cycle_params[0]){
		current_progress[0] = 100 - progress;
		current_progress[1] = 67 - progress;
		current_progress[2] = 34 - progress;
	}else if(current_counter >  cycle_params[0] && current_counter <= cycle_params[1]){
		current_progress[0] = 100 - progress;
		current_progress[1] = 67 - progress;
		current_progress[2] = progress - 33;
	}else if(current_counter >  cycle_params[1] && current_counter <= cycle_params[2]){
		current_progress[0] = 100 - progress;
		current_progress[1] = progress - 66;
		current_progress[2] = progress - 33;
	}else if(current_counter >  cycle_params[2] && current_counter <= cycle_params[3]){
		current_progress[0] = progress - 100;
		current_progress[1] = progress - 66;
		current_progress[2] = progress - 33;
	}else if(current_counter >  cycle_params[3] && current_counter <= cycle_params[4]){
		current_progress[0] = progress - 100;
		current_progress[1] = progress - 66;
		current_progress[2] = 234 - progress;
	}else {
		current_progress[0] = progress - 100;
		current_progress[1] = 267 - progress;
		current_progress[2] = 234 - progress;
	}
	for (i = 0; i < 3; ++i)
	{
		lumins_level_logic_prepared[i] = current_progress[i] / fade_in_out_fine_grained_level;
		lumins_level_deviation[i] = current_progress[i] - lumins_level_logic_prepared[i] * fade_in_out_fine_grained_level;
	}
}
static void
cycle_reached_bi_directional_pulsing(u16 cycle)
{
	// 最大周期
	if(cycle >= cycle_params[5])
	{
		// 计完成周期
		if(display_mode_set == combination || display_mode_set == sensor)
			finish_cycle_counter ++;
		reset_counter_for_cycle();
	}
}


// 8
static void 
bi_directional_chasing_driver(void) {
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}

static void
bi_directional_chasing_driver_init() {
	cycle_output[0] = ON;
	cycle_output[1] = OFF;
	cycle_output[2] = OFF;
	current_interval = 0;
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}


static void 
cycle_reached_bi_directional_chasing(u16 cycle) {

	if(cycle >= cycle_params[speed_level])
	{
		if(current_interval == 0){
			cycle_output[0] = ON;
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
			cycle_output[1] = ON;
			cycle_output[2] = ON;
		}
		else if(current_interval == 3){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = ON;
		}
		else if(current_interval == 4){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 5){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 6){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 7){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = ON;
		}
		else if(current_interval == 8){
			cycle_output[0] = OFF;
			cycle_output[1] = ON;
			cycle_output[2] = ON;
		}
		else if(current_interval == 9){
			cycle_output[0] = ON;
			cycle_output[1] = ON;
			cycle_output[2] = ON;
		}
		else if(current_interval == 10){
			cycle_output[0] = ON;
			cycle_output[1] = ON;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 11){
			cycle_output[0] = ON;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 12){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 13){
			cycle_output[0] = ON;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 14)
		{
			// 计完成周期
			if(display_mode_set == combination || display_mode_set == sensor)
				finish_cycle_counter ++;
			current_interval = 0;
		}
	}
}


// 9
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
		// 计完成周期
		if(display_mode_set == combination || display_mode_set == sensor)
				finish_cycle_counter ++;
		reverse_dim_in_out_flag();
		reset_counter_for_cycle();
	}
}


// 10
static void 
steady_flash_driver(void) {
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}

static void
steady_flash_driver_init() {
	cycle_output[0] = ON;
	cycle_output[1] = ON;
	cycle_output[2] = ON;
	current_interval = 0;
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}

static void 
cycle_reached_steady_flash(u16 cycle) {
	u8 i;
	if(cycle >= cycle_params[speed_level])
	{
		for(i = 0; i < 3; i++)
		{
			if(cycle_output[i]) cycle_output[i] = OFF;
			else cycle_output[i] = ON;
		}
		// 计完成周期
		if(display_mode_set == combination || display_mode_set == sensor)
			finish_cycle_counter ++;
		reset_counter_for_cycle();
	}
}

// 11
static void 
sequential_flashing_driver()
{
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}

static void
sequential_flashing_driver_init() {
	cycle_output[0] = ON;
	cycle_output[1] = ON;
	cycle_output[2] = ON;
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
			cycle_output[0] = OFF;
			cycle_output[1] = ON;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 1){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = ON;
		}
		else if(current_interval == 5){
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = ON;
		}

		else if(current_interval == 6){
			cycle_output[0] = OFF;
			cycle_output[1] = ON;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 7){
			cycle_output[0] = ON;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		else if(current_interval == 11){
			cycle_output[0] = ON;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		else
		{
			cycle_output[0] = OFF;
			cycle_output[1] = OFF;
			cycle_output[2] = OFF;
		}
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 12)
		{
			// 计完成周期
			if(display_mode_set == combination || display_mode_set == sensor)
				finish_cycle_counter ++;
			current_interval = 0;
		}
	}
}

// 12
// progressive_storbing
static void 
progressive_strobing_driver(void) {
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}

static void
progressive_strobing_driver_init() {
	cycle_output[0] = ON;
	cycle_output[1] = ON;
	cycle_output[2] = ON;
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
	current_interval = 0;
	strobing_half_cycle_counter = strobing_half_cycle_counter_per_interval[current_interval];
}

static void 
cycle_reached_progressive_strobing(u16 cycle) {
	u8 i;
	if(cycle >= cycle_params[current_interval] >> 1)
	{
		for(i = 0; i < 3; i++)
		{
			if(cycle_output[i]) cycle_output[i] = 0;
			else cycle_output[i] = 1;
		}
		reset_counter_for_cycle();
		--strobing_half_cycle_counter;
		if (!strobing_half_cycle_counter) {
			++ current_interval;
			if(current_interval == 9)
			{
				// 计完成周期
				if(display_mode_set == combination || display_mode_set == sensor)
					finish_cycle_counter ++;
				current_interval = 0;
			}
			strobing_half_cycle_counter = strobing_half_cycle_counter_per_interval[current_interval];
		}

	}
}

// 13
// bi_directional_storbing
static void 
bi_directional_storbing_driver(void) {
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}

static void
bi_directional_storbing_driver_init() {
	cycle_output[0] = ON;
	cycle_output[1] = OFF;
	cycle_output[2] = OFF;
	cycle_output_DUTY[0] = ON;
	cycle_output_DUTY[1] = OFF;
	cycle_output_DUTY[2] = OFF;
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
	current_interval = 0;
	strobing_half_cycle_counter = bi_directional_storbing_half_cycle_counter_per_interval[current_interval];
}

static void 
cycle_reached_bi_directional_storbing(u16 cycle) {
	u8 i;
	if(cycle >= cycle_params[current_interval] >> 1)
	{
		for(i = 0; i < 3; i++)
		{
			if(cycle_output_DUTY[i] == OFF)
			{
				cycle_output[i] = 0;
			}
			else
			{
				if(cycle_output[i]) cycle_output[i] = 0;
				else cycle_output[i] = 1;
			}
			
		}
		if(current_interval == 0)
		{
			cycle_output_DUTY[0] = ON;
			cycle_output_DUTY[1] = ON;
			cycle_output_DUTY[2] = OFF;
		}
		else if(current_interval == 1)
		{
			cycle_output_DUTY[0] = ON;
			cycle_output_DUTY[1] = ON;
			cycle_output_DUTY[2] = ON;
		}
		else if(current_interval == 2)
		{
			cycle_output_DUTY[0] = ON;
			cycle_output_DUTY[1] = ON;
			cycle_output_DUTY[2] = ON;
		}
		else if(current_interval == 3)
		{
			cycle_output_DUTY[0] = ON;
			cycle_output_DUTY[1] = ON;
			cycle_output_DUTY[2] = ON;
		}
		else if(current_interval == 4)
		{
			cycle_output_DUTY[0] = ON;
			cycle_output_DUTY[1] = ON;
			cycle_output_DUTY[2] = OFF;
		}
		else if(current_interval == 5)
		{
			cycle_output_DUTY[0] = ON;
			cycle_output_DUTY[1] = OFF;
			cycle_output_DUTY[2] = OFF;
		}
		else if(current_interval == 6)
		{
			cycle_output_DUTY[0] = OFF;
			cycle_output_DUTY[1] = OFF;
			cycle_output_DUTY[2] = OFF;
		}
		else if(current_interval == 7)
		{
			cycle_output_DUTY[0] = ON;
			cycle_output_DUTY[1] = OFF;
			cycle_output_DUTY[2] = OFF;
		}
        --strobing_half_cycle_counter;
		if (!strobing_half_cycle_counter) {
			++ current_interval;
			if(current_interval == 8 || (display_mode_set == sensor && current_interval == 4))
			{
				current_interval = 0;
				// 计完成周期
				if(display_mode_set == combination || display_mode_set == sensor)
					finish_cycle_counter ++;
			}
			strobing_half_cycle_counter = strobing_half_cycle_counter_per_interval[current_interval];
		}
		reset_counter_for_cycle();

	}
}
// 14
// fade_in_fade_out
static void 
fade_in_fade_out_driver(void) {
	u8 i;
	u16 current_counter = counter_for_cycle;
	for(i = 0; i < 3; i++)
	{
		u16 progress = (current_counter * 100UL) / (cycle_for_fade_in_fade_out[speed_level]) ;
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
fade_in_fade_out_driver_init() {
	cycle_output[0] = ON;
	cycle_output[1] = ON;
	cycle_output[2] = ON;
	fade_in_out_flag[0] = RISE;
	fade_in_out_flag[1] = RISE;
	fade_in_out_flag[2] = RISE;	
	lumins_level_logic_prepared[0] = lumins_level_min;
	lumins_level_logic_prepared[1] = lumins_level_min;
	lumins_level_logic_prepared[2] = lumins_level_min;
}

static void 
cycle_reached_fade_in_fade_out(u16 cycle) {
	u8 i;
	if(cycle >= cycle_params[speed_level])
	{
		for(i = 0; i < 3; i++)
		{
			if(fade_in_out_flag[i]) fade_in_out_flag[i] = 0;
			else fade_in_out_flag[i] = 1;
		}
		// 计完成周期
		if(display_mode_set == combination || display_mode_set == sensor)
				finish_cycle_counter ++;
		reset_counter_for_cycle();
	}
}



// 15
// steady_burn
static void 
steady_burn_driver(void) {
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}

static void
steady_burn_driver_init() {
	cycle_output[0] = ON;
	cycle_output[1] = ON;
	cycle_output[2] = ON;
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}

/*
*	Sensor Mode
*/

static void 
sensor_driver() {
	u8 i;
	for(i = 0; i < 3; i++)
	{
		if(lumins_level_logic_prepared[i] < 1)
			lumins_level_logic_prepared[i] = 1;
	}
}

static void
sensor_driver_init() {
	current_interval = 0;
	cycle_output[0] = ON;
	cycle_output[1] = ON;
	cycle_output[2] = ON;
	lumins_level_logic_prepared[0] = lumins_level_set;
	lumins_level_logic_prepared[1] = lumins_level_set;
	lumins_level_logic_prepared[2] = lumins_level_set;
}

static void 
cycle_reached_sensor(u16 cycle)
{
	if(cycle >= cycle_params[current_interval])
	{
		if(current_interval == 0){
			lumins_level_logic_prepared[0] = lumins_level_set;
			lumins_level_logic_prepared[1] = lumins_level_set;
			lumins_level_logic_prepared[2] = lumins_level_set;
		}
		else if(current_interval == 1){
			lumins_level_logic_prepared[0] = lumins_level_set;
			lumins_level_logic_prepared[1] = lumins_level_set;
			lumins_level_logic_prepared[2] = lumins_level_set;
		}
		else if(current_interval == 2){
			lumins_level_logic_prepared[0] = lumins_level_set;
			lumins_level_logic_prepared[1] = lumins_level_set;
			lumins_level_logic_prepared[2] = lumins_level_set;
		}
		else if(current_interval == 3){
			lumins_level_logic_prepared[0] = lumins_level_set;
			lumins_level_logic_prepared[1] = lumins_level_set;
			lumins_level_logic_prepared[2] = lumins_level_set;
		}
		else if(current_interval == 4){
			lumins_level_logic_prepared[0] = lumins_level_set;
			lumins_level_logic_prepared[1] = lumins_level_set;
			lumins_level_logic_prepared[2] = lumins_level_set;
		}
		else if(current_interval == 5){
			lumins_level_logic_prepared[0] = lumins_level_set - 3;
			lumins_level_logic_prepared[1] = lumins_level_set - 3;
			lumins_level_logic_prepared[2] = lumins_level_set - 3;
		}

		else if(current_interval == 6){
			lumins_level_logic_prepared[0] = lumins_level_set;
			lumins_level_logic_prepared[1] = lumins_level_set;
			lumins_level_logic_prepared[2] = lumins_level_set;
		}
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 7)
			current_interval = 0;
	}
}