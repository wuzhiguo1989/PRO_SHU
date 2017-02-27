#include "led.h"
#include "utils.h"



void T1_code()
{
    LED_OUT = 1;
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    LED_OUT = 0;
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
}

void T0_code()
{
    LED_OUT = 1;
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    LED_OUT = 0;
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
}


// // G7G6G5G4G3G2G1G0R7R6R5R4R3R2R1R0B7B6B5B4B3B2B1B0
void led_color_8bit(led_t *led_config)
{
    u8 flag, iterator;
    u8 mask = 0x80, t_code = led_config -> green; 
    for(iterator = 0; iterator < 8; ++iterator)
    {
        flag = (t_code & mask) >> 7;
        if(flag == 1){
            T1_code();
        } else {
            T0_code();
        }
        t_code <<= 1;
    }

    t_code = led_config -> red;
    for(iterator = 0; iterator < 8; ++iterator)
    {
        flag = (t_code & mask) >> 7;
        if(flag == 1){
            T1_code();
        } else {
            T0_code();
        }
        t_code <<= 1;
    }

    t_code = led_config -> blue;
    for(iterator = 0; iterator < 8; ++iterator)
    {
        flag = (t_code & mask) >> 7;
        if(flag == 1){
            T1_code();
        } else {
            T0_code();
        }
        t_code <<= 1;
    }
}


void led_config_change()
{
    u8 led_index;
    for(led_index = 0; led_index < 19; led_index ++)
        led_color_8bit(&led_config_logic[led_index]);  
}
