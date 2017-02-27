#ifndef __LM567_H_
#define __LM567_H_

// max = 8
#define SAMP_PER_MS						4
// max = 8
#define SIG_LENGTH						8
// max = 16
#define SIG_WIDTH						10
// 
#define LM567_STAT						P33

extern volatile u8 signal_ms_counter;
extern volatile u8 signal_bit_counter;
extern volatile u8 signal_length_counter;
extern volatile u8 io_status_per_ms;
extern volatile u16 io_status_per_bit;
extern volatile u8 signal_raw_data;

extern volatile u8 fall_counter_while_bit;
extern volatile u8 fall_counter_whole;
extern volatile u8 rise_counter_in_tail;
extern volatile u8 high_counter_in_tail;

extern volatile bool fall_counter_bit_take_account;
extern volatile bool candidate_signal_tail_check;

#define abort_recieving()	candidate_signal_recieving = NO;
#define quick_validate_signal(x)	((x & 0x01) && (x & 0x80))
#define effective_signal(x)	((x & 0x7E) >> 1)

void init_signal_decoder(void);
void signal_buffer_process(void);
void start_check_candidate_tail();

#endif