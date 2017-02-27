#ifndef __ADC_H_
#define __ADC_H_

#include "led.h"

#define ADC_POWER   0x80            //ADC电源控制位
#define ADC_FLAG    0x10            //ADC完成标志
#define ADC_START   0x08            //ADC起始控制位
#define ADC_SPEEDLL 0x00            //540个时钟
#define ADC_SPEEDL  0x20            //360个时钟
#define ADC_SPEEDH  0x40            //180个时钟
#define ADC_SPEEDHH 0x60            //90个时钟
#define ADC_SPEED 	ADC_SPEEDHH

void init_adc();
void adc_handler();
u16 read_adc_period_max();
void refresh_adc_period_max();
void adc_sampling();

#define adc_enable()	adc_should_sampling = YES
#define adc_disable()	adc_should_sampling = NO

#endif