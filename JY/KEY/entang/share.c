#include "share.h"
#include "utils.h"
volatile bool  candidate_time_out_signal_recieving = NO;
volatile bool  candidate_driver_signal_recieving = NO;
volatile u8 ENTANG_A_user_key_password[4] = {0x0F, 0x23, 0x65, 0xF8};