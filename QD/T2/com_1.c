/*	
*	STC-ISP导入芯片信息至Keil
*	采用STC15W404AS系列
*	假定测试芯片的工作频率为18.432MHz
*/
#include "intrins.h"
#include "led.h"
#include "uart.h"
#include "sync.h"
#include "adc.h"
#include "display.h"
#include "shared.h"
#include "lm567.h"
#include "nRF24L01.h"

#define disable_all_interrupt() EA = 0
#define enable_all_interrupt() EA = 1

static void init_system_component();
static void init_logic_params();
static void init_adc_amplifier();
static void adc_sampling_rest();

#define IO_ADC		0x02	// P1.1
#define IO_LED1     0x20	// P5.5
#define IO_LED2		0x10	// P5.4
#define IO_LED3     0x01	// P1.0
#define IO_INT		0x04	// P3.2
//#define IO_CODE		0x08	// P3.3

void Delay100ms()		//@18.432MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 8;
	j = 1;
	k = 243;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}


void require_delay_init() {
	//set_push_pull_output_on1(IO_LED3);
	set_push_pull_output_on5(IO_LED1);
	set_push_pull_output_on5(IO_LED2);
	//set_push_pull_output_on1(IO_AMP);
//	set_quasi_bidirectional_on1(IO_AMP);
//	set_input_only_on1(IO_ADC);
	set_input_only_on3(IO_INT);
//	set_input_only_on3(IO_CODE);

//	init_adc_amplifier();
//	init_adc();
}
void BLE_searched_init()
{
	MCU_CS = 1;
	sync_with_alert();
	MCU_CS = 0;
	sync_with_alert();
	MCU_CS = 1;
}

void BLE_modify_name()
{
	BT_RTS = 0;
	sync_with_alert();
	modify_name();
	Delay100ms();
	BT_RTS = 1;
}

void instant_effective_init() {
	init_system_component();
	init_logic_params();
}

void main()
{
	// stop catching interrupt
	disable_all_interrupt();
	// 
	require_delay_init();

	instant_effective_init();
	
	// BLE_modify_name();

	// -> sync.h
	BLE_searched_init();
	
	// // begin catching interrupt
	enable_all_interrupt();
	// run forever
	NRF24L01_Init();
	NRF24L01_TX_Mode(); 

    while(1) {
    //	BLE_modify_name();
    	// Feed dog
    	reset_watch_dog();
        if (INT_PROC & UART_PROC) {
    		parse_crtl_signal_from_BLE();
    		INT_PROC &= ~UART_PROC;
    		continue;
    	}
    	// io status sampling finished
    	if (INT_PROC & LM567_DUTY) {
    		signal_t signal_raw;
    		u8 signal_raw_search_index = 0, signal_raw_fall_counter = 2;
		    if(quick_validate_signal(signal_raw_data)) {
		    	while (signal_raw_search_index < 7) {
		    		if ((signal_raw_data & (0x03 << signal_raw_search_index)) == (0x02 << signal_raw_search_index))
		    		{
		    			++signal_raw_search_index;
		    			++signal_raw_fall_counter;
		    		}
		    		++signal_raw_search_index;
		    	}

		    	if (fall_counter_whole < signal_raw_fall_counter) {
		    		signal_raw = effective_signal(signal_raw_data);
					parse_crtl_signal(signal_raw);
		    	}
			}
			INT_PROC &= ~LM567_DUTY;
			EX1 = ENABLE;
    		continue;
    	}
    	// 60hz * 2
    	if (INT_PROC & EXINT_PROC) {
 //   		if (cycle_output && toggle_status) 
    		if (toggle_status) 
    			toggle_once();	// -> display.h
				else 
				{
					Tx2Buf0[0] = Tx2Buf0[1] = 0;
					Tx2Buf1[0] = Tx2Buf1[1] = 0;
					Tx2Buf2[0] = Tx2Buf2[1] = 0;
					INT_CLKO &= 0xEF; // 关闭发送中断
					NRF24L01_TX_Mode();
					NRF24L01_TxPacket_NUM(Tx2Buf0,0);
					NRF24L01_TxPacket_NUM(Tx2Buf1,1);
					NRF24L01_TxPacket_NUM(Tx2Buf2,2);
					NRF24L01_RX_Mode(4);
					INT_CLKO |= 0x10;
				}
    		prepare_next_display_params();
    		INT_PROC &= ~EXINT_PROC;
    		continue;
    	}
   //  	// 5ms system tick-tock
    	if (INT_PROC & TICK_DUTY) {
    		cycle_based_adjust(counter_for_cycle);
            INT_PROC &= ~TICK_DUTY;
    		continue;
    	}
    	if (INT_PROC & RECIEVE_DUTY) {
    		if(!NRF24L01_RxPacket(Rx2Buf))
    		{
    			signal_t signal_data = Rx2Buf[0];
    			parse_crtl_signal(signal_data);
    		}
    		INT_CLKO |= 0x10;             
    		INT_PROC &= ~RECIEVE_DUTY;
    		continue;
    	}
    	if (INT_PROC & MINUTE_DUTY) {
    		// -> display.h
			skip_to_next_display_if_should();
			INT_PROC &= ~MINUTE_DUTY;
			continue;
    	}
    	if (adc_should_sampling) {
    		adc_sampling();
    	}
    }
}

static void
init_adc_amplifier() {
	high(P14);
}

static void
init_system_component() {
	init_uart();
	init_sync_timer();
	init_ac_driver();
	init_phase_sync_timer();
	init_signal_decoder();
}

static void
init_logic_params() {
	display_startup_init();
}