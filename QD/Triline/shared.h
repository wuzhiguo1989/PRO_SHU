#include "def.h"
#include "stc15w408as.h"

extern volatile toggle_t toggle_status;
extern volatile disp_t display_mode_set, display_mode_logic;
extern volatile lumins_level_t lumins_level_set;
//lumins_level_logic, lumins_level_logic_prepared;
extern volatile lumins_level_t lumins_level_logic[3];
extern volatile lumins_level_t lumins_level_logic_prepared[3];
//
extern volatile speed_level_t speed_level;
//extern volatile bit cycle_output;
extern volatile u8 cycle_output[3];
extern volatile u8 cycle_output_DUTY[3];
extern volatile u16 counter_for_cycle;
extern volatile bit FLAG;
extern volatile u8 INT_PROC;
extern volatile BOOL adc_should_sampling;
extern volatile BOOL candidate_signal_recieving;
//extern volatile BOOL impluse_holding;
extern volatile u8 impluse_holding[3];
extern volatile u16 calibrated_phase_delay[3];
extern volatile u16 CCPA_tmp[3];