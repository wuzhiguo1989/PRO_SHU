#ifndef __UTILS_H_
#define __UTILS_H_

#include "stc15wxxxx.h"

#define high_at_bit(r, bit)		r |= bit
#define low_at_bit(r, bit)		r &= ~bit

#define set_quasi_bidirectional_on(x, pin) 		low_at_bit(P##x##M1, pin); low_at_bit(P##x##M0, pin)
#define set_push_pull_output_on(x, pin)			low_at_bit(P##x##M1, pin); high_at_bit(P##x##M0, pin)
#define set_input_only_on(x, pin)				high_at_bit(P##x##M1, pin); low_at_bit(P##x##M0, pin)
#define set_open_drain_on(x, pin)				high_at_bit(P##x##M1, pin); high_at_bit(P##x##M0, pin)

#define pure_input(x, pin)						set_input_only_on(x, pin)
#define push_pull(x, pin)						set_push_pull_output_on(x, pin)
#define quasi_bidirectional(x, pin)				set_quasi_bidirectional_on(x, pin)
#define open_drain(x, pin)						set_open_drain_on(x, pin)

#define ON 						1
#define OFF 					0
#define NO 						0
#define YES 					1

// flag set to 0
// clear set to 1
#define  WDT_FLAG	0x80
#define  CLR_WDT	0x10

#define reset_watch_dog()     WDT_CONTR &= ~WDT_FLAG;	WDT_CONTR |= CLR_WDT

void Store_Counter_for_Open_key_cabinet(u32 counter);

void Store_Counter_for_Open_flood_door(u32 counter);

void Store_Counter_for_Read_A_and_B(u32 counter);

void Store_Counter_for_Operate_A_B_and_Blename(u32 counter, u8 flag);

void Store_A(u8 *data_A, u8 data_from, u8 data_end);

void Store_B(u8 *data_B, u8 data_from, u8 data_end);



u32 Read_Counter_for_Open_key_cabinet();

u32 Read_Counter_for_Open_flood_door();

u32 Read_Counter_for_Read_A_and_B();

u32 Read_Counter_for_Write_A();

u32 Read_Counter_for_change_BLEname();

void Read_A(u8 *send_data, u8 data_from, u8 data_end);

void Read_B(u8 *send_data, u8 data_from, u8 data_end);



u8 CRC_Creat(u8 *receive, u8 start, u8 end);

bool ChecksimpleCRC(u8 *receive, u8 data_from, u8 data_end, u8 CRC_check);


u16 AND_Creat(u8 *receive, u8 data_from, u8 data_end);

bool CheckAnd(u8 *receive, u8 data_from, u8 data_end, u8 and_check_1, u8 and_check_2);

void Complement_Creat(u8 *receive, u8 data_from, u8 data_end, u8 *complemet_creat, u8 data_start, u8 data_over);

bool CheckComplement(u8 *receive, u8 data_from, u8 data_end, u8 *complement_check, u8 data_start, u8 data_over);

void Order_Over_Creat(u8 *receive, u8 data_from, u8 data_end, u8 *order_over_creat, u8 data_start, u8 data_over);

bool CheckOrder_Over(u8 *receive, u8 data_from, u8 data_end, u8 *order_over_check, u8 data_start, u8 data_over); 

void Exclusive_Or_Creat(u8 *receive_1, u8 data_from, u8 data_end, u8 *receive_2, u8 data_start, u8 data_over, u8 *exclusive_or_creat, u8 from, u8 end);


bool CheckExclusive_Or(u8 *receive_1, u8 data_from, u8 data_end, u8 *receive_2, u8 data_start, u8 data_over, u8 *exclusive_or_check, u8 from, u8 end);

#endif