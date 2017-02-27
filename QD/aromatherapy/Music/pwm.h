#ifndef __PWM_H_
#define __PWM_H_

#include "config.h"

#define PWMCLOCK            PwmClk_4T       // 1/4分频 考虑到要为15的倍数。
#define PWM_FIXED_FREQ      500             // 300Hz
// FIXED_CYCLE < PWM_MAX_CYCLE
#define PWM_FIXED_CYCLE     (MAIN_Fosc / 4 / PWM_FIXED_FREQ)
#define PWM_MAX_CYCLE       0x1000


typedef struct {
    u16 FirstEdge;              // 第一个翻转计数, 1~32767
    u16 SecondEdge;             // 第二个翻转计数, 1~32767
    u8  Start_IO_Level;         // 设置PWM输出端口的初始电平, 0或1
    u8  PWMx_IO_Select;         // PWM输出IO选择. PWM2_P37,PWM2_P27,PWM3_P21,PWM3_P45,PWM4_P22,PWM4_P44,PWM5_P23,PWM5_P42,PWM6_P16,PWM6_P07,PWM7_P17,PWM7_P06
    u8  PWMx_Interrupt;         // 中断允许,   ENABLE,DISABLE
    u8  FirstEdge_Interrupt;    // 第一个翻转中断允许, ENABLE,DISABLE
    u8  SecondEdge_Interrupt;   // 第二个翻转中断允许, ENABLE,DISABLE
} PWMx_InitDefine; 

#define PWMx_As_GPIO(PWM_id)       PWMCR &= ~(0x01 << PWM_id)
#define PWMx_As_PWM(PWM_id)        PWMCR |= (0x01 << PWM_id)

void PWM_Init();
void PWM_Pin_Init();
void PWM_SourceClk_Duty(const u8 PWM_SourceClk, const u16 PWM_Cycle);
void PWMx_Configuration(const u8 PWM_id, const PWMx_InitDefine xdata *PWMx);
void PWMx_Setup_Wide(const u8 PWM_id, const u16 width, const bool high);
void PWMx_Setup_DutyCycle(const u8 PWM_id, const u8 duty_cycle);
void PWMx_Setup_DutyCycle_Based(const u8 PWM_id, const u16 factor, const u16 base);

#endif
