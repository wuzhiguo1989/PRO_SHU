#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include "led.h"
typedef void (*cycle_reached_pt) (u8);
void toggle_once() ;										// called by exint0_isr
void cycle_based_adjust(const u16 cycle);					// called by pca_isr
void display_mode_set_changed();							// called by com_isr
void display_mode_logic_changed();							// called by pca_isr
#endif