#include "adc.h"
#include "intrins.h"
#include "alarm.h"
#include "share.h"
#include "ble.h"

#define wait_adc_finish() while (!(ADC_CONTR & ADC_FLAG))   //等待ADC转换完成
#define adc_bat_channel 6
#define adc_vac_channel 5


void init_adc()
{
	P1ASF |= 0x60;
	ADC_RES = 0;
	ADC_RESL = 0;
	ADC_CONTR = ADC_POWER | ADC_SPEED;
}


static u16 GetADCResult(u8 channel)
{
	ADC_RES = 0;
	ADC_RESL = 0;
	// 选通道需要清除之前的通道
	ADC_CONTR &= 0xF8;
	ADC_CONTR |= channel;
	ADC_CONTR |= ADC_START;
	_nop_();                        
    _nop_();
    _nop_();
    _nop_();
    wait_adc_finish();              
    ADC_CONTR &= ~ADC_FLAG;         
    return ((ADC_RES << 2) | ADC_RESL);
}


// 0:正常 1：有问题
static bool battery_detected()
{
	u8 i;
	u16 adc_value = 0;
	for(i = 0; i < 16; i++)
		adc_value += GetADCResult(adc_bat_channel);
	adc_value = adc_value >> 4;
	// TX1_write2buff(0XEE);
	// TX1_write2buff(adc_value);
	// TX1_write2buff(adc_value >> 8);
	if(adc_value > 235)
		return 0;	
	else 
		return 1;
}


// 0:正常 1：有问题
static bool vac_detected()
{
	u8 i;
	u16 adc_value = 0;
	for(i = 0; i < 16; i++)
		adc_value += GetADCResult(adc_vac_channel);
	adc_value = adc_value >> 4;
	// TX1_write2buff(0X88);
	// TX1_write2buff(adc_value);
	// TX1_write2buff(adc_value >> 8);
	if (adc_value > 20)
		return 0;	
	else 
		return 1;
}


void voltage_performance_detected()
{
	u8 battery_detected_flag, vac_detected_flag;
	battery_detected_flag = battery_detected();
	vac_detected_flag = vac_detected();
	// battery_detected_flag = 0;
	//vac_detected_flag = 0;
	// TX1_write2buff(battery_detected_flag);
	// TX1_write2buff(vac_detected_flag);
	// 电池电量低
	if(battery_detected_flag && !vac_detected_flag)
	{
		performance_status |= 0x80;
		performance_status &= ~0x40;

	}
	// 220V有问题
	if(!battery_detected_flag && vac_detected_flag)
	{
		performance_status |= 0x40;
		performance_status &= ~0x80;
		
	}
	if(!battery_detected_flag && !vac_detected_flag)
	{
		performance_status &= ~0xE0;
	}
	// 电压正常
	// if(!battery_detected_flag && !vac_detected_flag)
	// {	
	// 	performance_status &= 0x20;
	// 	// 锁有问题
	// 	if(performance_status & 0x20)
	// 		alarm_for_key_cabinet();
	// 	// 一切正常
	// 	else
	// 	{
	// 		alarm_for_no_problem();
	// 	} 
	// }	 
}
