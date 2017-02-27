#ifndef __BLE_H_
#define __BLE_H_

#include "config.h"

#define COM_TX1_Lenth   64
#define COM_RX1_Lenth   32

#define TimeOutSet1     50

void handle_uart_data();
void BLE_Init();
void init_uart();
void ble_process_recieved(char *received);
void BLE_ChangeBLEName(u8 *blename);
void BLE_Enter_Adversting();
void request_process_recieved(char *received);
void send_performance_status();
void TX1_write2buff(const u8 dat);
#endif