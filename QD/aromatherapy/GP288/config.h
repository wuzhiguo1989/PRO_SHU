#ifndef     __CONFIG_H_
#define     __CONFIG_H_

/*********************************************************/

#include    "stc15wxxxx.h"


// MCU pin PWM
sbit PWM2 = P3^7;
sbit PWM5 = P2^3;
sbit PWM7 = P1^7;
sbit PWM3 = P2^1;

#define PWM(x) (PWM##x)

#define PWM_LED 			PWM5_ID		
#define PWM_CTL				PWM7_ID
#define PWM_FENG            PWM3_ID

#define PWM_LED_Mx 				2
#define PWM_CTL_Mx				1
#define PWM_FENG_Mx             2

#define PWM_LED_BIT 			0x08
#define PWM_CTL_BIT				0x80
#define PWM_FENG_BIT            0x02

/***********************************************************/

#endif
