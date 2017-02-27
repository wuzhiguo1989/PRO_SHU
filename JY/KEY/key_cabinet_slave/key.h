#ifndef __KEY_H_
#define __KEY_H_

#include "config.h"

/*****************************************************
    行列键扫描程序
    使用XY查找5*6键的方法，只能单键，速度快

   Y     KA1      KA2      KA3      KA4      KA5
          |        |        |        |		    |
X         |        |        |        |        |
KB1 ---- K00 ---- K06 ---- K12 ---- K18 ---- K24
          |        |        |        |        |
KB2 ---- K01 ---- K07 ---- K13 ---- K19 ---- K25
          |        |        |        |        |
KB3 ---- K02 ---- K08 ---- K14 ---- K20 ---- K26 
          |        |        |        |        |
KB4 ---- K03 ---- K09 ---- K15 ---- K21 ---- K27
          |        |        |        |        |
KB5 ---- K04 ---- K10 ---- K16 ---- K22 ---- K28
		      |        |        |        |        |
KB6 ---- K05 ---- K11 ---- K17 ---- K23 ---- K29
		      |        |        |        |        |
******************************************************/
#define KA1         P12
#define KA2         P13
#define KA3         P14
#define KA4         P17
#define KA5			    P55
#define KA6					P50

#define KB1         P40
#define KB2         P64
#define KB3         P65
#define KB4         P66
#define KB5         P67
#define KB6         P32

#define KA1_Mx      1
#define KA2_Mx      1
#define KA3_Mx      1
#define KA4_Mx      1
#define KA5_Mx      5
#define KA6_Mx      3

#define KB1_Mx      4
#define KB2_Mx      6
#define KB3_Mx      6
#define KB4_Mx      6
#define KB5_Mx      6
#define KB6_Mx      3
          
#define KA1_BIT      0x04
#define KA2_BIT      0x08
#define KA3_BIT      0x10
#define KA4_BIT      0x80
#define KA5_BIT      0x20
#define KA6_BIT      0x01


#define KB1_BIT      0x01
#define KB2_BIT      0x10
#define KB3_BIT      0x20
#define KB4_BIT      0x40
#define KB5_BIT      0x80
#define KB6_BIT      0x04

#define LONG_KEY_TIME   250
#define SHORT_KEY_TIME  50
u32 IO_KeyScan_G();
void key_scan_task();
void key_duty_task();

#endif