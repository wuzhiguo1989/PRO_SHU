#include "share.h"
#include "utils.h"
volatile bool  candidate_time_out_signal_recieving = NO;
volatile bool  candidate_driver_signal_recieving = NO;
volatile u8 key_cabinet_number = 1; 
volatile u8 performance_status = 0;
volatile u8 order_for_badkey[30];
volatile u8 number_for_badkey = 0;