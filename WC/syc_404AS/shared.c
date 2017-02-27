#include "shared.h"

// Default is STEADY
volatile disp_t display_mode_set = steady_burn, display_mode_logic = steady_burn;
// Default is 100, logic is for output, driver change _prepared
volatile u8 cycle_output[2] = {ON, ON};
// 5ms counters
volatile u16 counter_for_cycle = 0;
volatile bit cycle_finished = 0;
volatile u32 time_counter = 0;
// SYSTEM ALL INT MASK
volatile u8 INT_PROC = 0; 