#ifndef __LM567_H_
#define __LM567_H_


#include "config.h"  
 
#define COM_TX1_Lenth   64 
#define COM_RX1_Lenth   32  
 
#define TimeOutSet1     2 
 
 
void uart_process_recieved(); 
void BLE_Init(); 
void TX1_write2buff(const u8 dat);  


#endif