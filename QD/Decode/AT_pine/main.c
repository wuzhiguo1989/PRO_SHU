#include "intrins.h"
#include "stc15wxxxx.h"
#include "bk8000l.h"
#include "string.h"
//-----------------------------------------------

#define BAUD    9600
#define FOSC    24000000L
#define T0INT   (65536-FOSC/4000)             // 8000次采样 / 秒
#define T2INT   (65536 - (FOSC/4/BAUD))       // 

#define ENABLE          1
#define DISABLE         0
#define YES             1
#define NO              0
#define false 0
#define true 1

#define SAMP_PER_MS                     4
#define SIG_LENGTH                      7
#define SIG_WIDTH                       6
#define LM567_STAT                      P33
#define IO_CODE                         0x08    // P3.3


#define high_at_bit(r, bit)     r |= bit
#define low_at_bit(r, bit)      r &= ~bit

#define set_quasi_bidirectional_on3(pin)        low_at_bit(P3M1, pin); low_at_bit(P3M0, pin)
#define set_push_pull_output_on3(pin)           low_at_bit(P3M1, pin); high_at_bit(P3M0, pin)
#define set_input_only_on3(pin)                 high_at_bit(P3M1, pin); low_at_bit(P3M0, pin)
#define set_open_drain_on3(pin)                 high_at_bit(P3M1, pin); high_at_bit(P3M0, pin)
#define push_back(ori, x) ori <<= 1; if(x) ++ori;

#define abort_recieving() candidate_signal_recieving = NO;
#define quick_validate_signal(x)  ((x & 0x01) && (x & 0x40))
#define effective_signal(x) ((x & 0x3E) >> 1)

// #define validate_signal(x)  ((x & 0x01) && (x & 0x40))
// #define effective_signal(x) ((x & 0x3E) >> 1)
// #define send_command(x)   PrintString1(x)

void init_signal_decoder(void);
void signal_buffer_process(void);
void start_check_candidate_tail();

// static volatile u8 signal_ms_counter = SAMP_PER_MS;
// static volatile u8 signal_bit_counter = SIG_WIDTH;
// static volatile u8 signal_length_counter = SIG_LENGTH;
// static volatile u8 io_status_per_ms = 0;
// static volatile u16 io_status_per_bit = 0;
// static volatile u8 signal_raw_data = 0;
static volatile bit LM567_DUTY = 0;
static volatile bit candidate_signal_recieving = NO;


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

u8 command_counter = 0;
bit TIME_DUTY = 0;
bit MODE_DUTY = 0;
bit LIMUS_DUTY = 0;
bit COLOR_DUTY = 0;
bit ALARM1_DUTY = 0;
bit ALARM2_DUTY = 0;
bit ALARM_DUTY = 0;
bit SPEED_DUTY = 0;
u8 syc_data_raw[15];

void InitTimer0()
{
    TMOD = 0x00;                    // 低两位 = 0 —— 模式0(16位自动重装载)
    AUXR |= 0x80;                   // 定时器0的速度是传统单片机的12倍（1T）
    TL0 = T0INT;                    // 初始化计时值
    TH0 = T0INT >> 8;
    TR0 = 1;                        // 定时器0开始计时
    ET0 = 1;                        // 使能定时器0中断
}

void 
InitExINT1(void) {
    IT1 = 0;                        //  设置INT1的中断类型 (1:仅下降沿 0:上升沿和下降沿)
    PX1 = 1;
    EX1 = ENABLE;                   //  使能INT1中断
}

// static void init_recieving() {
// 	signal_ms_counter = SAMP_PER_MS;
// 	signal_bit_counter = SIG_WIDTH;
// 	signal_length_counter = SIG_LENGTH;
// 	io_status_per_ms = 0;
// 	io_status_per_bit = 0;
// 	signal_raw_data = 0;

// 	fall_counter_while_bit = 0;
// 	fall_counter_whole = 0;
// 	rise_counter_in_tail = 0;
// 	high_counter_in_tail = 0;

// 	candidate_signal_recieving = YES;
// 	candidate_signal_tail_check = false;
// 	fall_counter_bit_take_account = false;
// }

// void start_check_candidate_tail() {
// 	candidate_signal_recieving = NO;
// 	candidate_signal_tail_check = true;
// }

// void signal_buffer_process(void) {
// 	candidate_signal_tail_check = false;

// 	if (rise_counter_in_tail < 2 && high_counter_in_tail < 2) {
// 		LM567_DUTY = 1;
// 		EX1 = DISABLE;
// 	}
// }
void 
init_recieving() {
    signal_ms_counter = SAMP_PER_MS;
    signal_bit_counter = SIG_WIDTH;
    signal_length_counter = SIG_LENGTH;
    io_status_per_ms = 0;
    io_status_per_bit = 0;
    signal_raw_data = 0;
    candidate_signal_recieving = YES;
}

void signal_buffer_process(void) {
    candidate_signal_recieving = NO;
    LM567_DUTY = 1;
}

void parse_ctrl(u8 receive_data)
{
	  if(receive_data == 16 && !command_counter)
    {
       TIME_DUTY = 1;
       memset(syc_data_raw, 0, 15);
       syc_data_raw[command_counter] = receive_data;
       command_counter ++;
    }
		else if(receive_data == 17 && command_counter == 3)
    {
       TIME_DUTY = 1;
       command_counter ++;
    }
    else if(receive_data == 18 && command_counter == 6)
    {
        TIME_DUTY = 1;
        command_counter ++;
    }
    else if(receive_data == 19 && command_counter == 9)
    {
        TIME_DUTY = 1;
        command_counter ++;
    }
    else if(receive_data == 20 && command_counter == 12)
		{
        TIME_DUTY = 1;
        command_counter ++;
		}
    else if(receive_data == 21 && !command_counter)
    {
         MODE_DUTY = 1;
         memset(syc_data_raw, 0, 15);
				 syc_data_raw[command_counter] = receive_data;
         command_counter ++;
    }
    else if(receive_data == 22 && !command_counter)
    {
         LIMUS_DUTY = 1;
			   memset(syc_data_raw, 0, 15);
				 syc_data_raw[command_counter] = receive_data;
         command_counter ++;
    }
    else if(receive_data == 23 && !command_counter)
    {
          COLOR_DUTY = 1;
          memset(syc_data_raw, 0, 15);
					syc_data_raw[command_counter] = receive_data;
          command_counter ++;
    }
    else if(receive_data == 24 && command_counter == 3)
    {
          COLOR_DUTY = 1;
          command_counter ++;
    }
    else if(receive_data == 25 && command_counter == 6)
    {
          COLOR_DUTY = 1;
          command_counter ++;
    }
		else if(receive_data == 26 && !command_counter)
    {
          ALARM1_DUTY = 1;
					memset(syc_data_raw, 0, 15);
					syc_data_raw[command_counter] = receive_data;
          command_counter ++;
    }
		else if(receive_data == 27 && command_counter == 3)
    {
          ALARM1_DUTY = 1;
          command_counter ++;
    }
		else if(receive_data == 28 && !command_counter)
    {
          ALARM2_DUTY = 1;
					memset(syc_data_raw, 0, 15);
					syc_data_raw[command_counter] = receive_data;
          command_counter ++;
    }
		else if(receive_data == 29 && command_counter == 3)
    {
          ALARM2_DUTY = 1;
          command_counter ++;
    }
		else if(receive_data == 30 && !command_counter)
    {
         SPEED_DUTY = 1;
			   memset(syc_data_raw, 0, 15);
				 syc_data_raw[command_counter] = receive_data;
         command_counter ++;
    }
    else if(receive_data == 31 && !command_counter)
    {
         ALARM_DUTY = 1;
         memset(syc_data_raw, 0, 15);
         syc_data_raw[command_counter] = receive_data;
         command_counter ++;
    }
    else if((receive_data >= 0 && receive_data <= 15)  && (command_counter % 3))
    {
          if(TIME_DUTY)
          {
             syc_data_raw[command_counter] = receive_data;
						 command_counter ++;
             if(!(command_counter % 3))
                 TIME_DUTY = 0;
              if(command_counter == 15)
              {
                  Bk8000l_Sync_Time(syc_data_raw[1] << 4 | syc_data_raw[2], 
                                    syc_data_raw[4] << 4 | syc_data_raw[5], 
                                    syc_data_raw[7] << 4 | syc_data_raw[8], 
                                    syc_data_raw[10] << 4 | syc_data_raw[11], 
                                    syc_data_raw[13] << 4 | syc_data_raw[14]);
                  command_counter = 0;
              }
           }
           if(MODE_DUTY)
           {
               syc_data_raw[command_counter] = receive_data;
               command_counter ++;
               if(command_counter == 2)
               {
                  Bk8000l_Change_DisplayMode(syc_data_raw[1]);
                  command_counter = 0;
                  MODE_DUTY = 0;
               }
           }
           if(LIMUS_DUTY)
           {
                syc_data_raw[command_counter] = receive_data;
                command_counter ++;
                if(command_counter == 2)
                {
                     Bk8000l_Change_DisplayLimus(syc_data_raw[1]);
                     command_counter = 0;
                     LIMUS_DUTY = 0;
                }
            }
            if(COLOR_DUTY)
            {
                syc_data_raw[command_counter] = receive_data;
                command_counter ++;
                if(!(command_counter % 3))
                    COLOR_DUTY = 0;
                if(command_counter == 9)
                {
                    Bk8000l_Sync_Color(syc_data_raw[1] << 4 | syc_data_raw[2], 
                                    syc_data_raw[4] << 4 | syc_data_raw[5], 
                                    syc_data_raw[7] << 4 | syc_data_raw[8]);
                    command_counter = 0;
                }
            }
						if(ALARM1_DUTY)
            {
                syc_data_raw[command_counter] = receive_data;
                command_counter ++;
                if(!(command_counter % 3))
                    ALARM1_DUTY = 0;
                if(command_counter == 6)
                {
                    Bk8000l_Sync_ALARM1(syc_data_raw[1] << 4 | syc_data_raw[2], 
                                    syc_data_raw[4] << 4 | syc_data_raw[5]);
                    command_counter = 0;
                }
            }
						if(ALARM2_DUTY)
            {
                syc_data_raw[command_counter] = receive_data;
                command_counter ++;
                if(!(command_counter % 3))
                    ALARM2_DUTY = 0;
                if(command_counter == 6)
                {
                    Bk8000l_Sync_ALARM2(syc_data_raw[1] << 4 | syc_data_raw[2], 
                                    syc_data_raw[4] << 4 | syc_data_raw[5]);
                    command_counter = 0;

                }
            }
					 if(SPEED_DUTY)
           {
                syc_data_raw[command_counter] = receive_data;
                command_counter ++;
                if(command_counter == 2)
                {
                     Bk8000l_Change_DisplaySpeed(syc_data_raw[1]);
                     command_counter = 0;
                     LIMUS_DUTY = 0;
                }
            }
            if(ALARM_DUTY)
            {
                syc_data_raw[command_counter] = receive_data;
                command_counter ++;
                if(command_counter == 2)
                {
                     Bk8000l_Control_ALARM(syc_data_raw[1]);
                     command_counter = 0;
                     LIMUS_DUTY = 0;
                }
            }


            
      }
     else
     {
           command_counter = 0;
           TIME_DUTY = 0;
           MODE_DUTY = 0;
           LIMUS_DUTY = 0;
           COLOR_DUTY = 0;
           ALARM_DUTY = 0;
           SPEED_DUTY = 0;
           ALARM1_DUTY = 0;
           ALARM2_DUTY = 0;
    }

}

void main()
{
    set_input_only_on3(IO_CODE);
    InitTimer0();
    InitExINT1();
    Bk8000l_Init();
    EA = 1;
    while(1) 
    {
				if (LM567_DUTY) 
        {
					u8 signal_raw;
//					u8 signal_raw_search_index = 0, signal_raw_fall_counter = 2;
					if(quick_validate_signal(signal_raw_data)) 
          {
                signal_raw = effective_signal(signal_raw_data);
//                Bk8000l_Change_RedColor(signal_raw);
                parse_ctrl(signal_raw);
//               while (signal_raw_search_index < 4) {
// 							   if ((signal_raw_data & (0x03 << signal_raw_search_index)) == (0x02 << signal_raw_search_index))
// 							   {
// 								    ++signal_raw_search_index;
// 								    ++signal_raw_fall_counter;
// 							   }
// 							   ++signal_raw_search_index;
//               }
//               if (fall_counter_whole < signal_raw_fall_counter) {
//                  signal_raw = effective_signal(signal_raw_data);
//                 Bk8000l_Change_RedColor(signal_raw);
// //						    parse_ctrl(signal_raw);
//               }
 				  }
          LM567_DUTY = 0;
          EX1 = ENABLE;
        }
    }
}

// LM567 caught one candidate signal
void exint1_isr() interrupt 2 using 2 
{
//    Check INT is RISE
   if (LM567_STAT) {
       EX1 = DISABLE;
       init_recieving();
   }
}


// void exint1_isr() interrupt 2 using 2 
// {
//  // Check INT is RISE
//  if (LM567_STAT && !candidate_signal_recieving && !candidate_signal_tail_check) {
//    // begin to sampling from P33
//    init_recieving();
//  }

//  if (LM567_STAT && candidate_signal_tail_check)
//    ++rise_counter_in_tail;

//  if (!LM567_STAT && candidate_signal_recieving) {
//    if (fall_counter_bit_take_account)
//      ++fall_counter_while_bit;

//    ++fall_counter_whole;
//  }
// }

static u8 io_status_per_ms_checker = 0;
static u8 io_status_rf_per_ms = 0;

void tm0_isr() interrupt 1 using 1
{
    if (candidate_signal_recieving) {
        --signal_ms_counter;
        push_back(io_status_per_ms, LM567_STAT);
        if (!signal_ms_counter) {
            // 1ms finish
            signal_ms_counter = SAMP_PER_MS;
            --signal_bit_counter;
            if(signal_length_counter == 7)
            {
              if(signal_bit_counter >= 3)
              {
                if(io_status_per_ms == 0x0F)
                  io_status_per_ms = 1;
                else
                {
                  candidate_signal_recieving = NO;
                  EX1 = ENABLE;
                }
              }
            }
            push_back(io_status_per_bit, io_status_per_ms);
            if(signal_length_counter == 7)
            {
              if(signal_bit_counter == 3)
              {
                if(io_status_per_bit != 0x07)
                { 
                  candidate_signal_recieving = NO;
                  EX1 = ENABLE;
                }
              }
            } 
            io_status_per_ms = 0;
            if (!signal_bit_counter) {
                // 10ms / 1bit finish
                signal_bit_counter = SIG_WIDTH;
                --signal_length_counter;
                push_back(signal_raw_data, (io_status_per_bit & 0x08
							));
                io_status_per_bit = 0;
                if (!signal_length_counter) {
                    // 8bit finish
                    signal_buffer_process();
                }
            }
        }
    }
	}
		
// 		if (candidate_signal_tail_check) {
// 		--signal_ms_counter;
// 		if (LM567_STAT && signal_bit_counter < 6)
// 			++high_counter_in_tail;
		
// 		if (!signal_ms_counter) {						// 1ms
// 			signal_ms_counter = SAMP_PER_MS;
// 			--signal_bit_counter;
// 			if (!signal_bit_counter) {					// 10ms / 1bit finish
// 				signal_bit_counter = SIG_WIDTH;
// 				signal_buffer_process();
// 			}
// 		}
// 	}

// 	// 4kHz new tick-tock
// 	if (candidate_signal_recieving) {
// 		--signal_ms_counter;

// 		push_back(io_status_per_ms, LM567_STAT);
// 		if (!signal_ms_counter) {
// 			// 1ms finish
// 			signal_ms_counter = SAMP_PER_MS;
// 			--signal_bit_counter;

// 			// ------------- check four io status in one ms ----------------
// 			io_status_per_ms_checker = io_status_per_ms << 1;
// 			io_status_per_ms_checker ^= io_status_per_ms;

// 			io_status_rf_per_ms = 0;
// 			if (io_status_per_ms_checker & 0x02)
// 				++io_status_rf_per_ms;
// 			if (io_status_per_ms_checker & 0x04)
// 				++io_status_rf_per_ms;
// 			if (io_status_per_ms_checker & 0x08)
// 				++io_status_rf_per_ms;
// 			if (io_status_per_ms_checker & 0x10)
// 				++io_status_rf_per_ms;

// 			if (io_status_rf_per_ms > 2)
// 				abort_recieving();

// 			// ------------- check four io status in one ms ----------------

// 			if (signal_bit_counter == 5) {
// 				fall_counter_bit_take_account = true;
// 				fall_counter_while_bit = 0;
// 			}
// 			if (signal_bit_counter == 1) {
// 				fall_counter_bit_take_account = false;
// 			}

// 			push_back(io_status_per_bit, io_status_per_ms);
// 			io_status_per_ms = 0;
// 			if (!signal_bit_counter) {
// 				// 10ms / 1bit finish
// 				signal_bit_counter = SIG_WIDTH;
// 				--signal_length_counter;

// 				io_status_per_bit &= 0x0C;
// 				if (io_status_per_bit && io_status_per_bit != 0x0C && fall_counter_while_bit) {
// 					abort_recieving();
// 				}

// 				push_back(signal_raw_data, io_status_per_bit);
// 				io_status_per_bit = 0;
// 				if (!signal_length_counter) {
// 					// 8bit finish
// 					start_check_candidate_tail();
// 				}
// 			}
		
//     }
//   }
// }




