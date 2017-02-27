#include "adc.h"
#include "intrins.h"
#include "share.h"
#include "key.h"
#include "utils.h"
#include "74hc595.h"
#include "sync.h"

#define wait_adc_finish() while (!(ADC_CONTR & ADC_FLAG))   //等待ADC转换完成
#define adc_vbg_channel   9
#define adc_down_channel  4
#define adc_up_channel    3
#define adc_lock_channel  6
#define adc_detect_channel 7

#define ADC_CAL_COUNT    16
#define ADC_KEY_INPUT    18

u8 xdata index_adc_up = 0; 
u8 xdata index_adc_down = 0; 
u8 xdata index_adc_lock = 0;

u16 xdata key_up_adc[ADC_CAL_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
u16 xdata key_down_adc[ADC_CAL_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; 
u16 xdata key_lock_adc[ADC_CAL_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

u16 xdata key_up_sum, key_down_sum, key_lock_sum;
u16 xdata key_up_avg, key_down_avg, key_lock_avg;
u16 xdata key_up_now, key_down_now, key_lock_now;

static u16 GetADCResult(u8 channel)
{
  P1ASF &= ~0xFF;
  if(channel == 9)
    P1ASF |= 0;
  else 
    P1ASF |= (1 << channel);
	ADC_RES = 0;
	ADC_RESL = 0;
  // 选通道需要清除之前的通道
  ADC_CONTR &= 0xF8;
  ADC_CONTR = ADC_POWER | ADC_SPEED;
  _nop_();                        
  _nop_();
  if(channel == 9)
    ADC_CONTR |= 0;
  else 
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

void init_adc()
{
    u8 i;
    key_up_avg = key_down_avg = key_lock_avg = 0;
    key_up_sum = key_down_sum = key_lock_sum = 0;
    for(i = 0; i < ADC_CAL_COUNT; i++)
    {
      key_up_adc[i] = 0;
      key_down_adc[i] = 0;
      key_lock_adc[i] = 0;
    };
}


u8 ADC_KeyScan_G() {
    u8 key = 0xFF;
    u8 key_up = 0;
    u8 key_down = 0;
    u8 key_lock = 0;
    key_up_now = GetADCResult(adc_up_channel);
    if(key_up_now < key_up_avg && key_up_avg - key_up_now > ADC_KEY_INPUT)
      key_up = 0;
    else
    {
      key_up_sum += key_up_now;
      key_up_sum -= key_up_adc[index_adc_up];
      key_up_adc[index_adc_up] = key_up_now;
      key_up_avg = key_up_sum >> 4;
      index_adc_up ++;
      if(index_adc_up == ADC_CAL_COUNT)
        index_adc_up = 0;
      key_up = 1;
    }

    key = key << 1 | key_up;

    key_down_now = GetADCResult(adc_down_channel);
    if(key_down_now < key_down_avg && key_down_avg - key_down_now > ADC_KEY_INPUT)
      key_down = 0;
    else
    {
      key_down_sum += key_down_now;
      key_down_sum -= key_down_adc[index_adc_down];
      key_down_adc[index_adc_down] = key_down_now;
      key_down_avg = key_down_sum >> 4;
      index_adc_down ++;
      if(index_adc_down == ADC_CAL_COUNT)
        index_adc_down = 0;
      key_down = 1;
    }

    key = key << 1 | key_down;


    // key_lock_now = GetADCResult(adc_lock_channel);
    // if(key_lock_now < key_lock_avg && key_lock_avg - key_lock_now >  ADC_KEY_INPUT)
			 // key_lock = 0;
    // else
    // {
    //   key_lock_sum += key_lock_now;
    //   key_lock_sum -= key_lock_adc[index_adc_lock];
    //   key_lock_adc[index_adc_lock] = key_lock_now;
    //   key_lock_avg = key_lock_sum >> 4;
    //   index_adc_lock ++;
    //   if(index_adc_lock == ADC_CAL_COUNT)
    //     index_adc_lock = 0;
    //   key_lock = 1;
    // }

    // key = key << 1 | key_lock;

    return key;
}


static code u8 const power_level_count[5] = {27, 32, 35, 38, 41};

void power_level_cal()
{
	u16 battery = 0; 
	u16 adc_value = 0;
  //读出基准电压
	// for(i = 0; i < 16; i++)
	adc_value += GetADCResult(adc_vbg_channel);
	// adc_value = adc_value >> 4;
  battery = 12992 / adc_value;
  // battery = adc_value;
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
  if(!CHG)
    power_level = power_level - 1;
}


#define ADC_DETECT_EXIST 15
#define ADC_VCC 1024

u16 Cnt_exit = 0;
u8 adc_now_exit = 1;
u8 adc_pre_exit = 1;

#define TIME_CNT_EXIT 1000

void reset_cnt_exit()
{
    Cnt_exit = 0;
}

u16 Cnt_exit_display()
{
   return key_up_now;
}

void start_work_detect()
{
  u16 adc_vbg_value, adc_detect_value; 
  adc_vbg_value = adc_detect_value = 0;
  adc_detect_value = GetADCResult(adc_detect_channel);
  if(ADC_VCC - adc_detect_value > ADC_DETECT_EXIST)
  {
     // 外围线路在
      reset_work_start();
      countdown_signal_start = YES;
      adc_sampling = YES;
  }
  else
  {
     // 外围线路不在
      countdown_signal_start = NO;
      time_colon_icon = 1;
      adc_sampling = YES;
  } 
}



void no_work_exit()
{
  u16 adc_vbg_value, adc_detect_value; 
  adc_vbg_value = adc_detect_value = 0;
  adc_detect_value = GetADCResult(adc_detect_channel);
  if(ADC_VCC - adc_detect_value > ADC_DETECT_EXIST)
    // 有
      adc_now_exit = 1;
  else
    // 没有
      adc_now_exit = 0;

  if(!adc_now_exit && !adc_pre_exit){
      // 一直没有、等待插入
      // todo：一定时间到、休眠再讨论
      // Cnt_exit ++;
      // if(Cnt_exit > TIME_CNT_EXIT)
      // {
      //   // 直接退出
      //     Displayclean();
      //     charge_on = 0;
      //     countdown_signal_start = NO;
      //     time_colon_icon = 1;
      //     // POUT = 0;
      //     P31 = P12 = P55 = 1;
      //     key_sampling = NO;
      //     adc_sampling = NO;
      // }
  } else if(adc_now_exit && !adc_pre_exit){
      // 没有到有
      // 美牙程序已启动
      P31 = 0;
      P55 = 1;
      P12 = 1;
      Cnt_exit = 0;
      countdown_signal_start = YES;
  }else if(!adc_now_exit && adc_pre_exit){
      // 有到没有
      Cnt_exit = 0;
      P31 = P12 = P55 = 1;
      time_colon_icon = 1;
      countdown_signal_start = NO;
  } else {
      // 一直有
      Cnt_exit = 0;
  }
  adc_pre_exit  =  adc_now_exit;
}