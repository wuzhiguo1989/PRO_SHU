#ifndef __LM567_H_
#define __LM567_H_
#include "stc15wxxxx.h"

typedef enum uart_send_id{
  open_success = 0,
  open_fail,
  lock_success,
  lock_fail,
  beep_stop,
  trace_success,
  trace_test,
  status_inqury
} send_id;

extern volatile send_id uart_id; 

void uart_process_recieved(); 
void uart_Init(); 
void TX1_write2buff(const u8 dat);
void uart_send_open_success();
void uart_send_status_now();
void uart_send_open_fail();
void uart_send_lock_fail();
void uart_send_alter_beep_success();
void uart_send_lock_by_hand_success();
void uart_send_lock_by_hand_fail(); 
void uart_send_beep_stop();  
#endif