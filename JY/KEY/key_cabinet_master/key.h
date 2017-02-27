#ifndef __KEY_H_
#define __KEY_H_

#include "config.h"

#define K1   P52
#define K2   P04
#define K3   P03
#define K4   P02
#define K5	 P16
#define K6	 P17
#define K7   P46
#define K8   P45
#define K9   P77
#define K10  P76
#define K11  P75
#define K12  P74
#define K13  P27
#define K14  P26
#define K15  P25
#define K16  P24
#define K17  P23
#define K18  P22
#define K19  P21
#define K20  P20
#define K21  P44
#define K22  P43
#define K23  P42
#define K24  P41
#define K25  P73
#define K26  P72
#define K27  P71
#define K28  P70
#define K29  P35
#define K30  P51


#define LONG_KEY_TIME   600
#define SHORT_KEY_TIME  50

void key_io_init();
u32 IO_KeyScan_G();
void key_scan_task();
void key_duty_task();

#endif