#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include "led.h"

typedef void (*display_driver_pt) (void);
typedef void (*cycle_reached_pt) (u16);

#define lumins_level_min 		0
#define lumins_level_peace		2
#define lumins_level_max 		10

#define LUMINS_LEVEL_RAW_MAX 0x0b
#define SPEED_LEVEL_RAW_MAX 0x0b

// recieved toggle status x == 0x01 || x == 0x02
#define check_toggle_status(x) (x == OFF + 1 || x == ON + 1)
// recieved display mode x > 0 && x <= senser_mode(9)
#define check_display_mode_type(x) (x && !(x > sensor))
// recieved lumins x > 0 && x <= LUMINS_LEVEL_RAW_MAX
#define check_lumins_level(x) (x && !(x > LUMINS_LEVEL_RAW_MAX))
// recieved speed x > 0 && x <= SPEED_LEVEL_RAW_MAX
#define check_speed_level(x)  (x && !(x > SPEED_LEVEL_RAW_MAX))



// public declaring
void display_startup_init();								// called by main_isr
void prepare_next_display_params();							// called by exint0_isr
void reset_current_phase_delay_with(const hertz_t hz);		// called by pca_isr
void skip_to_next_display_if_should();						// called by pca_isr
void toggle_once() ;										// called by exint0_isr
void ctrl_display_mode_response(const disp_t prev);			// called by com_isr
void ctrl_toggle_status_response(const toggle_t prev);		// called by com_isr
void cycle_based_adjust(const u16 cycle);					// called by pca_isr
void display_mode_set_changed();							// called by com_isr
void display_mode_logic_changed();							// called by pca_isr

#define RISE 0
#define FALL 1

#define FULL 1
#define HALF 0

// x is belong to left-closed, right-open interval
#define belong_to_interval_lcro(x, left, right) (x >= left && x < right)
// x is belong to left-closed interval
#define belong_to_interval_lc(x, left) (x >= left)


#endif