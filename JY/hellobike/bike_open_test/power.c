#include "power.h"
#include "utils.h"
#include "task.h"
#include "beep.h"
#include "key.h"
#include "lm567.h"
#include "key.h"
 

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

    // P32
    push_pull(3, 0x04);

    k_power_disable();

    // push_pull(3, 0x08);

    // P33 = 0;

    // K1|P33
    pure_input(3, 0x08);

    INT1 = 0;
    // 低电平报警
    IT1 = 1;   //  设置INT0的中断类型 (1:仅下降沿 0:上升沿和下降沿)
    EX1 = DISABLE; 
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
volatile u8 detect_key_time_start = 0;
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


u8 read_index = 0;
u16 adc_value = 0;
volatile u8 read_status_signal = 0;
void init_lock_para(){
   read_index = 0;
   adc_value = 0;
   read_status_signal = 1; 
}

#define CNT_MAX_INDEX 16
void read_lock_status(){
    adc_value += GetADCResult();
    if(read_index == CNT_MAX_INDEX){
        if((adc_value >> 4) > ADC_VALUE_ON){
            status = off;
        } else {
            status = on;
        }
        if(!restart_read_status_signal){
            k_power_disable();
            //uart_send_status_restart();
            ++ cnt_restart;
            //reload_resend_restart_count_down();
            restart_read_status_signal = 1;
            // 重新发送
        }
        else
        {
            if(INT1 == 0 && status == off){
                uart_send_status_now(0x00, 0x01);
            } else if(INT1 == 1 && status == on){
                uart_send_status_now(0x11, 0x01);
            } else if(INT1 == 0 && status == on){
                uart_send_status_now(0x10, 0x01);
            } else if(INT1 == 1 && status == off){
                uart_send_status_now(0x01, 0x01);
            }


            reload_sleep_count_down();
        }
        read_status_signal = 0;
        k_power_disable();
    }
    ++ read_index;
}



void exint1() interrupt INT1_VECTOR  using 0
{
    EX1 = DISABLE;
    INT_PROC |= LOCK_DETECT_DUTY; 
}