#ifndef __POWER_H_
#define __POWER_H_
#include "stc15wxxxx.h"
#define IA_POWER_1 P12
#define IA_POWER_2 P13
#define IB_POWER_1 P11
#define IB_POWER_2 P10
// K2 中断检测  
#define K2   P15
#define K1   P33
#define K_POWER P37
#define CPU_WAKE_UP P32

#define ADC_POWER   0x80            //ADC电源控制位
#define ADC_FLAG    0x10            //ADC完成标志
#define ADC_START   0x08            //ADC起始控制位
#define ADC_SPEEDLL 0x00            //540个时钟
#define ADC_SPEEDL  0x20            //360个时钟
#define ADC_SPEEDH  0x40            //180个时钟
#define ADC_SPEEDHH 0x60            //90个时钟
#define ADC_SPEED 	ADC_SPEEDLL

#define power_open_work()     IA_POWER_1 = IA_POWER_2 = 1; IB_POWER_1 = IB_POWER_2 = 0
#define power_lock_work()     IA_POWER_1 = IA_POWER_2 = 0; IB_POWER_1 = IB_POWER_2 = 1
#define power_work_disable()  IA_POWER_1 = IA_POWER_2 = IB_POWER_1 = IB_POWER_2 = 0
#define k_power_enable()  K_POWER = 1
#define k_power_disable() K_POWER = 0
#define cpu_wake_up_enable()  CPU_WAKE_UP = 0
#define cpu_wake_up_disable()  CPU_WAKE_UP = 1

extern volatile u8 read_status_signal;
extern volatile u8 cnt_off_time_plus;
extern volatile u8 cnt_on_time_plus;
extern volatile u8 detect_key_time_start;
void power_io_init();
void alter_beep_protect();
void init_adc();
u8 status_lock_detect(); 
u8 status_lock_on_detect();
u8 status_lock_off_detect();
void read_lock_status();
void init_lock_para();
#endif
