#include "shared.h"

// Default is ON
volatile toggle_t toggle_status = ON;
// Default is STEADY
volatile disp_t display_mode_set = steady_burn, display_mode_logic = steady_burn;
// Default is 100, logic is for output, driver change _prepared
volatile lumins_level_t lumins_level_set = 10; 
volatile lumins_level_t lumins_level_logic[3] = {10};
volatile lumins_level_t lumins_level_logic_prepared[3] = {10};
// Default is 50
volatile speed_level_t speed_level = 5;
//
//volatile bit cycle_output = ON;	
//volatile bit cycle_output[3] = {ON, ON, ON};
volatile u8 cycle_output[3] = {ON, ON, ON};
volatile u8 cycle_output_DUTY[3] = {ON, ON, ON};
// 5ms counters
volatile u16 counter_for_cycle = 0;
volatile u8 finish_cycle_counter = 0;
// INT0 status: 1-Rise 0-Fall  
volatile bit FLAG;
// SYSTEM ALL INT MASK
volatile u8 INT_PROC = 0;
// 
volatile BOOL adc_should_sampling = NO;
// INT1: RISE -> candidate_signal_recieving: No -> Yes
volatile BOOL candidate_signal_recieving = NO;
// TIMER0 toggle_once -> first Int(impluse_holding: No -> Yes) -> second Int(impluse_holding: Yes -> No)
volatile u8 impluse_holding[3] = {NO, NO, NO};
volatile u16 calibrated_phase_delay[3] = {1, 1, 1};
volatile u16 CCPA_tmp[3] = {1, 1, 1};
volatile u8 Rx2Buf[1]= {0};	 // 
volatile u8 Tx2Buf0[2] = {0};
volatile u8 Tx2Buf1[2] = {0};
volatile u8 Tx2Buf2[2] = {0};

// volatile u16    CCPA0_tmp = calibrated_phase_delay[0];
// volatile u16    CCPA1_tmp = calibrated_phase_delay[1];
// volatile u16    CCPA2_tmp = calibrated_phase_delay[2];