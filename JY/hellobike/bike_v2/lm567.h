#ifndef __LM567_H_
#define __LM567_H_
#include "stc15wxxxx.h"


extern volatile u16 cnt_lock;
extern volatile u8 cnt_restart;
extern volatile u8 cnt_lock_by_use;
extern volatile u8 lock_from_control;
 
void uart_process_recieved(); 
void uart_Init(); 
void TX1_write2buff(const u8 dat);
void uart_send_open_success();
void uart_send_open_fail();
void uart_send_lock_fail();
void uart_send_lock_success();
void uart_send_alter_beep_success();
void uart_send_lock_by_hand_success();
void uart_send_lock_by_hand_fail();
void uart_send_schedule_callback();
void uart_send_status_now(u8 lock_status, u8 sys_status);
void uart_send_status_restart(); 
#endif