#ifndef     __ADC_H_
#define     __ADC_H_

#include "stc15wxxxx.h"

#define ADC_POWER   0x80            //ADC电源控制位
#define ADC_FLAG    0x10            //ADC完成标志
#define ADC_START   0x08            //ADC起始控制位
#define ADC_SPEEDLL 0x00            //540个时钟
#define ADC_SPEEDL  0x20            //360个时钟
#define ADC_SPEEDH  0x40            //180个时钟
#define ADC_SPEEDHH 0x60            //90个时钟
#define ADC_SPEED 	ADC_SPEEDHH

#define BATTERY      P13
#define POWER_IN_OUT P12

 
void init_adc();
u8 power_detect_in();
void power_level_cal();


#endif