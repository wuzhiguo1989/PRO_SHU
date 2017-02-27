#include "def.h"
#include "stc15w408as.h"

extern volatile disp_t display_mode_set, display_mode_logic;
extern volatile u8 cycle_output[2];
extern volatile u8 counter_for_cycle;
extern volatile bit cycle_finished;
extern volatile u32 time_counter;
extern volatile u8 INT_PROC;