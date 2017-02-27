#ifndef __LED_H_
#define __LED_H_

#include	"config.h"
#include	"PWM.h"

typedef unsigned char color_t;
typedef unsigned char lumins_level_t;

typedef struct {
	color_t red;
	color_t green;
	color_t blue;
	lumins_level_t lumins;
} led_t;


#define LED_OUT  P23
#define lumins_level_min 		0
#define lumins_level_max 		3

extern volatile led_t xdata led_config_set[19], led_config_logic[19];


// 256 color * 4 lumins 
void led_color_8bit(led_t *led_config);

void led_config_change();

#endif
