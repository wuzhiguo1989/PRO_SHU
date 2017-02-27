#include "led.h"
#include "lm567.h"
#include "shared.h"

volatile u8 signal_ms_counter = SAMP_PER_MS;
volatile u8 signal_bit_counter = SIG_WIDTH;
volatile u8 signal_length_counter = SIG_LENGTH;
volatile u8 io_status_per_ms = 0;
volatile u16 io_status_per_bit = 0;
volatile u8 signal_raw_data = 0;

volatile u8 fall_counter_while_bit = 0;
volatile u8 fall_counter_whole = 0;
volatile u8 rise_counter_in_tail = 0;
volatile u8 high_counter_in_tail = 0;

volatile bool fall_counter_bit_take_account = false;
volatile bool candidate_signal_tail_check = false;


static void init_recieving() {
	signal_ms_counter = SAMP_PER_MS;
	signal_bit_counter = SIG_WIDTH;
	signal_length_counter = SIG_LENGTH;
	io_status_per_ms = 0;
	io_status_per_bit = 0;
	signal_raw_data = 0;

	fall_counter_while_bit = 0;
	fall_counter_whole = 0;
	rise_counter_in_tail = 0;
	high_counter_in_tail = 0;

	candidate_signal_recieving = YES;
	candidate_signal_tail_check = false;
	fall_counter_bit_take_account = false;
}

void init_signal_decoder(void) {
    IT1 = 0;                    	//	设置INT1的中断类型 (1:仅下降沿 0:上升沿和下降沿)
    PX1 = 1;
    EX1 = DISABLE;                  
}

void start_check_candidate_tail() {
	candidate_signal_recieving = NO;
	candidate_signal_tail_check = true;
}

void signal_buffer_process(void) {
	candidate_signal_tail_check = false;

	if (rise_counter_in_tail < 2 && high_counter_in_tail < 2) {
		INT_PROC |= LM567_DUTY;
		EX1 = DISABLE;
	}
}

// LM567 caught one candidate signal
void exint1_isr() interrupt 2 using 2 
{
	// Check INT is RISE
	if (LM567_STAT && !candidate_signal_recieving && !candidate_signal_tail_check) {
		// begin to sampling from P33
		init_recieving();
	}

	if (LM567_STAT && candidate_signal_tail_check)
		++rise_counter_in_tail;

	if (!LM567_STAT && candidate_signal_recieving) {
		if (fall_counter_bit_take_account)
			++fall_counter_while_bit;

		++fall_counter_whole;
	}
}