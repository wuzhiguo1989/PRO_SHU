#ifndef     __ADC_H_
#define     __ADC_H_

#include "stc15w4k32s4.h"
#include "typealias.h"

#define ADC_POWER   0x80            //ADC电源控制位
#define ADC_FLAG    0x10            //ADC完成标志
#define ADC_START   0x08            //ADC起始控制位
#define ADC_SPEEDLL 0x00            //540个时钟
#define ADC_SPEEDL  0x20            //360个时钟
#define ADC_SPEEDH  0x40            //180个时钟
#define ADC_SPEEDHH 0x60            //90个时钟
#define ADC_SPEED 	ADC_SPEEDHH


#define PHASE_1  				P13
#define PHASE_2  				P14
#define PHASE_3  				P15
#define TEMPERATURE_FOR_AROUND  P12
#define TEMPERATURE_FOR_FAN     P16

extern u8 tem_around;
extern u8 tem_fan;
 
void init_adc();
u8 temperature_around_cal();
u8 temperature_fan_cal();
bool overload_detected();
#endif