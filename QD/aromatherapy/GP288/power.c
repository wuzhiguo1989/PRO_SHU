#include "power.h"
#include "utils.h"
#include "task.h"
volatile bool power_status = OFF;
volatile bool sys_status = OFF;
volatile power_level_t xdata power_level_set;
 

void power_config_init() {
    power_status = OFF;
    power_level_set = 0;
}


void power_overload_protect(){
    INT0 = 1;
    IT0 = 1;                    //  设置INT0的中断类型 (1:仅下降沿 0:上升沿和下降沿)
    EX0 = ENABLE; 
}


void power_level_8bit(const power_level_t xdata power_level) 
{
	if(power_level == 4){
    	PWMx_Setup_DutyCycle(PWM_CTL, 95);
    	// PWMx_Setup_DutyCycle(PWM_FENG, 100);
        PWM3 = 1;
    }else if(power_level == 3){
    	PWMx_Setup_DutyCycle(PWM_CTL, 90);
        PWM3 = 1;
    	// PWMx_Setup_DutyCycle(PWM_FENG, 100);
    }else if(power_level == 2){
    	PWMx_Setup_DutyCycle(PWM_CTL, 80);
        PWM3 = 1;
    	// PWMx_Setup_DutyCycle(PWM_FENG, 100);
    }else if(power_level == 1){
    	PWMx_Setup_DutyCycle(PWM_CTL, 70);
        PWM3 = 1;
    	// PWMx_Setup_DutyCycle(PWM_FENG, 100);
    }else if(power_level == 0){
    	// PWMx_Setup_DutyCycle(PWM_CTL, 0);
        PWMx_As_GPIO(PWM7_ID);
        PWM7 = 0;
        PWM3 = 0;
    	// PWMx_Setup_DutyCycle(PWM_FENG, 0);
    }
}

void power_select_level()
{
	if(power_status == ON)
		power_level_8bit(power_level_set);
	else
		power_level_8bit(0);
}


// overload protection
// 中断0
void exint0() interrupt INT0_VECTOR  using 3
{
    if(!INT0){
        INT_PROC |= PROTECT_DUTY;
    }
}