#include "adc.h"
#include "intrins.h"
#include "share.h"
#include "jl17467pqw.h"

#define wait_adc_finish() while (!(ADC_CONTR & ADC_FLAG))   //等待ADC转换完成
#define adc_tem_channel 2
#define adc_bat_channel 3

void init_adc()
{
	P1ASF |= 0x0C;
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

static u16 code temperature_table[91] =
{
	17180, 16140, 15180, 14280, 13440, 12670, 11940, 11270, 10630, 10040, 9488,
	8963, 8471, 8010, 7578, 7172, 6790, 6432, 6096, 5779, 5481, 5197,
	4928, 4675, 4435, 4209, 3995, 3793, 3602, 3422, 3251, 3094, 2945, 
	2804, 2670, 2543, 2422, 2370, 2199, 2095, 1997, 1904, 1816, 1732,  
	1652, 1577, 1505, 1437, 1372, 1310, 1252, 1196, 1143, 1093, 1045,
	1000, 957, 916, 877, 839, 804, 770, 738, 707, 678, 650,
	624, 598, 574, 551, 529, 508, 488, 469, 451, 433, 417,
	401, 386, 371, 357, 344, 331, 319, 307, 296, 285, 275,
	265, 255, 246
};

void temperature_detected()
{
	u8 i;
	// , a, b, mid;
	u16 temperature, adc_value = 0;
	// a = 0;
	// b = 90;
	for(i = 0; i < 32; i++)
		adc_value += GetADCResult(adc_tem_channel);
	adc_value = adc_value >> 5;
	// 计算temperature
	temperature = 20000000 / (10240000 / adc_value - 10000);
	// 查找
	for(i = 0; i < 91; i++)
	{
		if(temperature > temperature_table[0])
		{
			temperature = 0;
			break;
		}
		else if(temperature < temperature_table[90])
		{
			temperature = 90;
			break;
		}
		else if(temperature <= temperature_table[i] && temperature > temperature_table[i+1])
		{
			temperature = i;
			break;
		}

	}
	jt17467pqw_set_temperature(temperature);
}



void battery_detected()
{
	u8 i, battery;
	u16 adc_value = 0;
	for(i = 0; i < 16; i++)
		adc_value += GetADCResult(adc_bat_channel);
	adc_value = adc_value >> 4;
	battery = 105 * adc_value / 1024;

	// 计算
	jt17467pqw_set_battery(battery);
}