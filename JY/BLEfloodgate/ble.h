#ifndef __BK8000L_H_
#define __BK8000L_H_

#include "config.h"

#define COM_TX1_Lenth   64
#define COM_RX1_Lenth   32

#define TimeOutSet1     2

void BLE_Init();
void init_A(); 
void ble_process_recieved();
void BLE_Enter_Adversting();
void request_process_recieved();
#endif