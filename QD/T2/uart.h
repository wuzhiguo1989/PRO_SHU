#ifndef __UART_H_
#define __UART_H_

#include "led.h"
#define uart_enable() ES = 1
#define uart_disable() ES = 0

void init_uart();
void modify_name();
void parse_crtl_signal(const signal_t recieved_signal);
void parse_crtl_signal_from_BLE();
void parse_crtl_signal_BLE(const signal_t recieved_signal);

#endif