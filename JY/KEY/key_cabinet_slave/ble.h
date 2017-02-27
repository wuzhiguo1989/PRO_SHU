#ifndef __BLE_H_
#define __BLE_H_

#include "config.h"

#define uart_enable() ES = 1
#define uart_disable() ES = 0

#define TimeOutSet1     2

void Uart_Init();

#endif