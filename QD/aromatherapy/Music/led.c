#include "led.h"
#include "utils.h"


// 200Hz
// static code u16 const color[5][3]=
// {
//  {0,0,0},
//  {2500,5000,7500},
//  {5000,10000,15000},
//  {7500,15000,22500},
//  {10000,20000,30000}
// };


// 5：每一种灯光分为5级 即PWM占空比依次为0%、25%、50%、75%和100%。
// 3: 每一种占空比中亮度分为3级，依次占每一级占空比的1/3、2/3、1. 
volatile led_t xdata led_config_set;

#define COLOR_5LEVEL_LUMINS_4LEVEL      12
#define COLOR_8BIT_LUMINS_4LEVEL        765

void led_config_init(led_t xdata *led_config) {
    led_config -> red = 255;
    led_config -> green = 255;
    led_config -> blue = 255;
}

void led_config_cpy(led_t xdata *dest, const led_t xdata *src) {
    dest -> red = src -> red;
    dest -> green = src -> green;
    dest -> blue = src -> blue;
}



// 256 color * 4 lumins 
void led_color_8bit(const led_t xdata *led_config) 
{
    PWMx_Setup_DutyCycle_Based(PWM_R, led_config->red * 100 >> 8, 100);
    PWMx_Setup_DutyCycle_Based(PWM_G, led_config->green * 100 >> 8, 100);
    PWMx_Setup_DutyCycle_Based(PWM_B, led_config->blue * 100 >> 8, 100);
}



