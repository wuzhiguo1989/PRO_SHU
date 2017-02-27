/*	
*	STC-ISP导入芯片信息至Keil
*	采用STC15W404AS系列
*	假定测试芯片的工作频率为18.432MHz
*/

#include "led.h"
#include "uart.h"
#include "sync.h"
#include "adc.h"
#include "display.h"
#include "shared.h"
#include "lm567.h"
#include "intrins.h"

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
#define IO_CODE		0x08	// P3.3
#define IO_SENSOR   0x04    // P1.2


void Delay1000ms()		//@18.432MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 71;
	j = 10;
	k = 171;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}



void require_delay_init() {
	set_push_pull_output_on1(IO_LED3);
	set_push_pull_output_on5(IO_LED1);
	set_push_pull_output_on5(IO_LED2);

	//set_push_pull_output_on1(IO_AMP);
//	set_quasi_bidirectional_on1(IO_AMP);
	set_input_only_on1(IO_SENSOR);
	set_input_only_on1(IO_ADC);
	set_input_only_on3(IO_INT);
	set_input_only_on3(IO_CODE);

//	init_adc_amplifier();
	init_adc();
}

void instant_effective_init() {
	init_system_component();
	init_logic_params();
}

void main()
{
	u8 k = 8;
	// stop catching interrupt
	disable_all_interrupt();
	// 
	require_delay_init();
	
	while(--k)
	{
		Delay1000ms();
	}
	
	// -> sync.h
	sync_with_alert();

	instant_effective_init();
	

	
	

	// begin catching interrupt
	enable_all_interrupt();
	// run forever
	
	
    while(1) {
    	// Feed dog
    	reset_watch_dog();
        if (INT_PROC & UART_PROC) {
    		signal_t signal_raw = recieve_data_from_buffer();
    		parse_crtl_signal(signal_raw);
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
	// init_uart();
	init_sync_timer();
	init_ac_driver();
	init_phase_sync_timer();
	init_signal_decoder();
}

static void
init_logic_params() {
	display_startup_init();
}