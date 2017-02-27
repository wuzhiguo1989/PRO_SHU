#include "power.h"
#include "utils.h"
#include "task.h"
#include "beep.h"
#include "key.h"
#include "lm567.h"
#include "key.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
 

void power_io_init() {
    // ia
    push_pull(1, 0x04);
    push_pull(1, 0x08);
    // ib
    push_pull(1, 0x01);
    push_pull(1, 0x02);

    power_work_disable();

}

#define wait_adc_finish() while (!(ADC_CONTR & ADC_FLAG))   //等待ADC转换完成
#define adc_key_channel  5
// 2V
// #define ADC_VALUE_ON 0x026C 
// 1V
#define ADC_VALUE_ON 0x0136
void init_adc()
{
    P1ASF |= 0x20;                      // P1(ch) -> P1 ^ 5
    ADC_RES = 0;                        // high 8 bit       
    ADC_RESL = 0;                       // low 2 bit
    ADC_CONTR = ADC_POWER | ADC_SPEED | adc_key_channel;
}

static u16 GetADCResult()
{
  ADC_CONTR |= ADC_START;
  _nop_();                        
  _nop_();
  _nop_();
  _nop_();
  wait_adc_finish();              
  ADC_CONTR &= ~ADC_FLAG;

  return ((ADC_RES << 2) | ADC_RESL);
}



// alter
void alter_beep_protect()
{
    // K2|P15
    pure_input(1, 0x40);

    // K_POWER|P37
    push_pull(3, 0x80);

    // K1|P33
    pure_input(3, 0x08);

    // P32
    push_pull(3, 0x04);

    k_power_disable();
 

    INT1 = 0;
    // 低电平报警
    IT1 = 1;   //  设置INT0的中断类型 (1:仅下降沿 0:上升沿和下降沿)
    EX1 = ENABLE; 
}

u8 status_lock_detect()
{
    u16 key_adc_value = 0;
    // 检测ADC
    key_adc_value = GetADCResult();
    // 低电平是开
    // 高电平是关
    if(key_adc_value > ADC_VALUE_ON){
        return 0;
    } else {
        return 1;
    }
}




u8 key_status_on = 0;
u8 key_status_on_pre = 0;
volatile u8 cnt_on_time_plus = 0;
u8 status_lock_on_detect()
{
    u16 key_adc_value = GetADCResult();
    // TX1_write2buff(key_adc_value >> 8);
    // TX1_write2buff(key_adc_value);
    // 低电平是开
    // 高电平是关
    if(key_adc_value > ADC_VALUE_ON)
        key_status_on = 1;
    else
        key_status_on = 0;
    if(!key_status_on && !key_status_on_pre){
        cnt_on_time_plus ++;
    } else {
        cnt_on_time_plus = 0;
    }
    key_status_on_pre = key_status_on;
    return cnt_on_time_plus;
}

u8 key_status_off = 0;
u8 key_status_off_pre = 0;
volatile u8 cnt_off_time_plus = 0;
u8 status_lock_off_detect()
{
    u16 key_adc_value = GetADCResult();
    // TX1_write2buff(key_adc_value >> 8);
    // TX1_write2buff(key_adc_value);
    // 低电平是开
    // 高电平是关
    if(key_adc_value > ADC_VALUE_ON)
        key_status_off = 1;
    else
        key_status_off = 0;

    if(key_status_off && key_status_off_pre){
        cnt_off_time_plus ++;
    } else {
        cnt_off_time_plus = 0;
    }
    key_status_off_pre = key_status_off;
    return cnt_off_time_plus;
}


void exint1() interrupt INT1_VECTOR  using 0
{
    EX1 = DISABLE;
    INT_PROC |= LOCK_DETECT_DUTY; 
}

void send_adc()
{
    u16 key_adc_value = 0;
    // 检测ADC
    key_adc_value = GetADCResult();
    if(!INT1)
        printf("%d,1,\r\n",key_adc_value);
    else 
        printf("%d,0,\r\n",key_adc_value);
}