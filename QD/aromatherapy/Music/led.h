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
} led_t;

#define lumins_level_min 		0
#define lumins_level_max 		3

extern volatile led_t xdata led_config_set;

void led_config_init(led_t xdata *led_config);
void led_config_cpy(led_t xdata *dest, const led_t xdata *src);
bool led_config_cmp(const led_t xdata *left, const led_t xdata *right);

// 5 color * 4 lumins
void led_color_simple(const led_t xdata *led_config);

// 256 color * 4 lumins 
void led_color_8bit(const led_t xdata *led_config);

#endif
