#ifndef __LED_H_
#define __LED_H_

#include "config.h"

// MCU pin display
#define DISPLAY_0  P53
#define DISPLAY_1  P05
#define DISPLAY_2  P06
#define DISPLAY_3  P07
#define DISPLAY_4  P60
#define DISPLAY_5  P62
#define DISPLAY_6  P61
//  com口
#define DISPLAY_DG1  P63
#define DISPLAY_DG2  P47
#define DISPLAY_D(i)  DISPLAY_##i


void display_io_init();
void display_error(u8 number);
void no_error_display();
void led_display();
#endif