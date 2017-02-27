#ifndef     __CONFIG_H_
#define     __CONFIG_H_

/*********************************************************/

#include    "stc15wxxxx.h"


// MCU pin ble (TX <> RX)
#define BT_RTS  P35
#define BT_CTS  P34
#define MCU_CS	P33


#define BLE_TX_Mx	3
#define BLE_RX_Mx	3

#define BLE_TX_BIT	0x80
#define BLE_RX_BIT	0x40

// MCU pin PWM
sbit PWM3 = P2^1;
sbit PWM4 = P2^2;
sbit PWM5 = P2^3;
sbit PWM6 = P1^6;
sbit PWM7 = P1^7;

#define PWM(x) (PWM##x)

#define PWM_R 				PWM3_ID
#define PWM_G 				PWM4_ID
#define PWM_B 				PWM5_ID
// #define PWM_SELECT			
#define PWM_CTL				PWM6_ID

#define PWM_R_Mx 				2
#define PWM_G_Mx 				2
#define PWM_B_Mx 				2
// #define PWM_SELECT_Mx			1
#define PWM_CTL_Mx				1

#define PWM_R_BIT 				0x02
#define PWM_G_BIT 				0x04
#define PWM_B_BIT 				0x08
// #define PWM_SELECT_BIT			0x40
#define PWM_CTL_BIT				0x40

/***********************************************************/

#endif
