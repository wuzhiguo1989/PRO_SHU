#include "power.h"
#include "utils.h"

volatile bool power_status;
volatile power_level_t xdata power_level;
 

void power_config_init() {
    power_status = OFF;
    power_level = 4;
    POWER_IN = 0;  
    push_pull(0,0x01);
    pure_input(1, 0x80);
    pure_input(3, 0x04);
    // INT0 = 1;
    // IT0 = 0;                    //  设置INT0的中断类型 (1:仅下降沿 0:上升沿和下降沿)
    // EX0 = ENABLE; 
}

void power_switch_on_off(u8 on)
{
	if(!on)
	{
		power_status = OFF;
		POWER_IN = 0;
	}
	else
	{
		power_status = ON;
		POWER_IN = 1;
	}
}

void power_level_8bit(const power_level_t xdata power_config) 
{
    if(power_config == 4)
    	PWMx_Setup_DutyCycle(PWM_CTL, 100);
    else if(power_config == 3)
    	PWMx_Setup_DutyCycle(PWM_CTL, 58);
    else if(power_config == 2)
    	PWMx_Setup_DutyCycle(PWM_CTL, 57);
    else if(power_config == 1)
    	PWMx_Setup_DutyCycle(PWM_CTL, 56);
    else if(power_config == 0)
    	PWMx_Setup_DutyCycle(PWM_CTL, 0);
}

void power_select_level(power_level_t power_level_set)
{
	power_level = power_level_set;
	power_level_8bit(power_level);
}



void power_detect()
{
	if(POWER_OUT)
	{
		power_status = OFF;
		POWER_IN = 0;
	}
}

//中断0
// void exint0() interrupt INT0_VECTOR  using 3 //INT1中断入口
// {
// 	if(INT0)
// 	{
// 		power_status = OFF;
// 		POWER_IN = 0;
// 	}
// }