#ifndef __UART_H_
#define __UART_H_

#include "led.h"

#define uart_enable() ES = 1
#define uart_disable() ES = 0

void init_uart();
signal_t recieve_data_from_buffer();
void send_raw_data(const u8 dat);
void parse_crtl_signal(const signal_t recieved_signal);

#endif