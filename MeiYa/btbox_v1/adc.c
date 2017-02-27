#include "adc.h"
#include "intrins.h"
#include "share.h"
#include "key.h"
#include "utils.h"

#define wait_adc_finish() while (!(ADC_CONTR & ADC_FLAG))   //等待ADC转换完成
#define adc_bat_channel 3
#define adc_power_in_out 2
#define ADC_POWER_IN   0x0060

void init_adc()
{
	// P12
  pure_input(1, 0x40);
  // P13
  pure_input(1, 0x80);

  P1ASF |= 0x0C;
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

static code u8 const power_level_count[5] = {27, 32, 34, 36, 38};

void power_level_cal()
{
	u8 i;
	u16 battery = 0; 
	u16 adc_value = 0;
	for(i = 0; i < 16; i++)
		adc_value += GetADCResult(adc_bat_channel);
	adc_value = adc_value >> 4;
	battery = 25 * 1024 / adc_value;
  if(battery < power_level_count[0]){
  	power_level = 0;
  } else if(battery >= power_level_count[0] && battery < power_level_count[1]){
  	power_level = 1;
  } else if(battery >= power_level_count[1] && battery < power_level_count[2]){
  	power_level = 2;
  } else if(battery >= power_level_count[2] && battery < power_level_count[3]){
  	power_level = 3;
  } else if(battery >= power_level_count[3] && battery < power_level_count[4]){
  	power_level = 4;
  } else if(battery >= power_level_count[4]){
  	power_level = 5;
  }
  // 充电时闪烁
  if(!CHG)
  {
      power_level = charge_power;
      charge_power ++;
      if(charge_power > 5)
        charge_power = 0;
  }
}


// 返回值：0充电 1充电完成 2有错误 3正常倒计时 4正常
u8 power_detect_in()
{
  u8 i; 
  u16 adc_value = 0;
  if(!STY || !CHG)
  {
    return 0;
  }
  else
  {
    for(i = 0; i < 16; i++)
      adc_value += GetADCResult(adc_power_in_out);
    adc_value = adc_value >> 4;
    // 短路
    if(adc_value < ADC_POWER_IN)
    {
      charge_power = 0;
      POUT = 0;
      return 2;
    }
    // 正常
    else
    {
      charge_power = 0;
      return 1;
    }
	}
}