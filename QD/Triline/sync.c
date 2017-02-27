#include "sync.h"
#include "display.h"
#include "shared.h"
#include "intrins.h"
#include "uart.h"
#include "adc.h"
#include "lm567.h"


#define PCA0 0
#define PCA1 1
#define PCA2 2
//static void impluse_hold();
//static void set_phase_delay(  hertz_t hertz);
// static void do_per_minute();
static BOOL minute_count_down();

static u8 counter_for_second = COUNTER_PER_SECOND;
static u8 counter_for_minute = SECONDS_PER_MINUTE;
static u8 counter_for_test_cycle = 30;
//sstatic u16 counter_for_hertz = 0;		// count to 60Hz * 2 * 60(s)
static u16 pca_next_target;
static hertz_t current_hertz = 60;		// Default is 60Hz
static u16 counter_for_hertz = 0;
static u8 counter_for_pca5ms = PCA5ms;

// #define ensure_impulse_off() switch_off(CCP0); switch_off(CCP1); switch_off(CCP2)
// 3路PCA控制灯光输出
// todo 
// PCA初始化
void init_phase_sync_timer() {
	CCON = 0;                       //初始化PCA控制寄存器
                                    //PCA定时器停止
                                    //清除CF标志
                                    //清除模块中断标志
    CL = 0;
    CH = 0;
    CMOD = 0x00;                    //设置PCA时钟源
                                    //禁止PCA定时器溢出中断
    CCAPM0 = 0x49;                  //PCA模块0为16位定时器模式
    CCAPM1 = 0x49;                  //PCA模块1为16位定时器模式
    CCAPM2 = 0x49;                  //PCA模块2为16位定时器模式
    PPCA = 1;
    CR = 0;
 //   CR = 1;                         //PCA定时器开始工作
}

void init_ac_driver() {
	INT0 = 1;
    IT0 = 0;                    //	设置INT0的中断类型 (1:仅下降沿 0:上升沿和下降沿)
    EX0 = ENABLE;                
}

// 0.25ms中断计时
void init_sync_timer()
{
	AUXR &= 0x7f;					// 12T
    TMOD &= 0xf0;                   // 模式0(16位自动重装载)	
    TL0 = 0x80;     //设置定时初值
    TH0 = 0xFE;     //设置定时初值
    TF0 = 0;
    TR0 = 1;                        // 定时器0开始计时
    ET0 = ENABLE;					// 
	PT0 = 0;						// 高优先级
}

void reset_all_counters() {
	reset_counter_for_minute();
	reset_counter_for_second();
	// reset_counter_for_hertz();
	reset_counter_for_cycle();
}

// static void 
// do_per_minute() {

// 	current_hertz = counter_for_hertz / SECONDS_PER_MINUTE;
// 	reset_counter_for_hertz();
// 	reset_current_phase_delay_with(current_hertz);	
// }

void sync_with_alert() {	// 4000ms @18.432MHz
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 54;
	j = 35;
	k = 163;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

static BOOL
minute_count_down() {
	--counter_for_second;
	if (!counter_for_second) {
    reset_counter_for_second();
		--counter_for_minute;
 		if (!counter_for_minute) {
 			reset_counter_for_minute();
 			return YES;
 		}
 	} 
 	return NO;
 }


void exint0_isr() interrupt 0 using 1  // follow POWER AC impluse 50Hz or 60Hz
{
    // VERY IMPORTANT DON'T DO ANYTHING BEFORE HERE
    phase_ctrl_disable();
    switch_off(CCP0); 
    switch_off(CCP1); 
    switch_off(CCP2);
    INT_PROC |= EXINT_PROC;
    FLAG = INT0;     // =0(Fall); =1(Rise)
		// calc new param
	// ++counter_for_hertz;
}

void pca_isr (void) interrupt 7 using 1
{
    if(CCF0)       //PCA模块0中断
    {
        CCF0 = 0;       //清PCA模块0中断标志
        if(calibrated_phase_delay[0])
        {
            if(impluse_holding[0])
            {
               switch_off(CCP0);
               impluse_holding[0] = NO;
            }
            else
            {
                switch_on(CCP0);
                CCPA_tmp[0] = CCPA_tmp[0] +  PHASE_WIDTH;
                CCAP0L = CCPA_tmp[0];
                CCAP0H = CCPA_tmp[0] >> 8;
                impluse_holding[0] = YES; 
            }
        }
        else
            switch_off(CCP0);

    }
    if(CCF1)       //PCA模块1中断
    {
        CCF1 = 0;       //清PCA模块2中断标志
        if(calibrated_phase_delay[1])
        {
            if(impluse_holding[1])
            {
               switch_off(CCP1);
							impluse_holding[1] = NO; 
            }
            else
            {
                switch_on(CCP1);
                CCPA_tmp[1]  = CCPA_tmp[1] +  PHASE_WIDTH;
                CCAP1L = CCPA_tmp[1] ;
                CCAP1H = CCPA_tmp[1]  >> 8;
                impluse_holding[1] = YES; 
            }
        }
        else
            switch_off(CCP1);
    }
    if(CCF2)       //PCA模块2中断
    {
        CCF2 = 0;       //清PCA模块2中断标志
        if(calibrated_phase_delay[2])
        {
            if(impluse_holding[2])
            {
               switch_off(CCP2);
               impluse_holding[2] = NO; 
            }
            else
            {
                switch_on(CCP2);
                CCPA_tmp[2]  = CCPA_tmp[2]  +  PHASE_WIDTH;
                CCAP2L = CCPA_tmp[2] ;
                CCAP2H = CCPA_tmp[2]  >> 8;
                impluse_holding[2] = YES; 
            }
        }
        else
            switch_off(CCP2);
    }
}

static u8 io_status_per_ms_checker = 0;
static u8 io_status_rf_per_ms = 0;

void tm0_isr() interrupt 1 using 3
{
	    // 5ms old tick-tock 
    --counter_for_pca5ms;
    if (!counter_for_pca5ms) {
    	INT_PROC |= TICK_DUTY;
    	if (minute_count_down()) {
    		INT_PROC |= MINUTE_DUTY;	    	
     		//do_per_minute();
		}
			++counter_for_cycle;
    	counter_for_pca5ms = PCA5ms;
	}

	if (candidate_signal_tail_check) {
		--signal_ms_counter;
		if (LM567_STAT && signal_bit_counter < 8)
			++high_counter_in_tail;
		
		if (!signal_ms_counter) {						// 1ms
			signal_ms_counter = SAMP_PER_MS;
			--signal_bit_counter;
			if (!signal_bit_counter) {					// 10ms / 1bit finish
				signal_bit_counter = SIG_WIDTH;
				signal_buffer_process();
			}
		}
	}

	// 4kHz new tick-tock
	if (candidate_signal_recieving) {
		--signal_ms_counter;

		push_back(io_status_per_ms, LM567_STAT);
		if (!signal_ms_counter) {
			// 1ms finish
			signal_ms_counter = SAMP_PER_MS;
			--signal_bit_counter;

			// ------------- check four io status in one ms ----------------
			io_status_per_ms_checker = io_status_per_ms << 1;
			io_status_per_ms_checker ^= io_status_per_ms;

			io_status_rf_per_ms = 0;
			if (io_status_per_ms_checker & 0x02)
				++io_status_rf_per_ms;
			if (io_status_per_ms_checker & 0x04)
				++io_status_rf_per_ms;
			if (io_status_per_ms_checker & 0x08)
				++io_status_rf_per_ms;
			if (io_status_per_ms_checker & 0x10)
				++io_status_rf_per_ms;

			if (io_status_rf_per_ms > 2)
				abort_recieving();

			// ------------- check four io status in one ms ----------------

			if (signal_bit_counter == 7) {
				fall_counter_bit_take_account = true;
				fall_counter_while_bit = 0;
			}
			if (signal_bit_counter == 3) {
				fall_counter_bit_take_account = false;
			}

			push_back(io_status_per_bit, io_status_per_ms);
			io_status_per_ms = 0;
			if (!signal_bit_counter) {
				// 10ms / 1bit finish
				signal_bit_counter = SIG_WIDTH;
				--signal_length_counter;

				io_status_per_bit &= 0x78;
				if (io_status_per_bit && io_status_per_bit != 0x78 && fall_counter_while_bit) {
					abort_recieving();
				}

				push_back(signal_raw_data, io_status_per_bit);
				io_status_per_bit = 0;
				if (!signal_length_counter) {
					// 8bit finish
					start_check_candidate_tail();
				}
			}
		}
	}
}