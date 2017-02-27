#ifndef __LM567_H_
#define __LM567_H_
#include "stc15wxxxx.h"
 
void uart_process_recieved(); 
void uart_Init(); 
char TX1_write2buff(const u8 dat);
void uart_send_open_success();
void uart_send_open_fail();
void uart_send_lock_fail();
void uart_send_alter_beep_success();
void uart_send_lock_by_hand_success();
void uart_send_lock_by_hand_fail();  
#endif