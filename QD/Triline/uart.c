#include "uart.h"
#include "sync.h"
#include "def.h"
#include "display.h"
#include "shared.h"
#include "intrins.h"

static BOOL busy = NO;
static u8 uart_buffer;

void init_uart() {
	SCON = 0x50;                // 设置串口为8位可变波特率
    T2L = 0x20;                 
    T2H = 0xfe;                 // 9600 bps
    AUXR |= 0x14;               // T2为1T模式, 并启动定时器2
    AUXR |= 0x01;               // 选择定时器2为串口1的波特率发生器
	PS = 1;						// 高优先级
	uart_enable();
}

// CAUTIONS! DON't CALL THIS rapidly
void send_raw_data(  u8 dat) {
	while (busy);                   //	等待前一个数据发送完成
    busy = YES;                      
    SBUF = dat;                     //	发送当前数据
}

signal_t recieve_data_from_buffer() {
	return uart_buffer;
}

/*----------------------------
Parse Control Signal
----------------------------*/
void 
parse_crtl_signal(  signal_t recieved_signal) {
	// local variable
	u8 dispatched_signal;
	ctrl_sig_t ctrl_signal;
	toggle_t prev_toggle_status = toggle_status;
	disp_t prev_display_mode_set = display_mode_set;

	uart_disable();
	// 控制信号 第1和第2位 
	dispatched_signal = (recieved_signal & 0x03);
	ctrl_signal = (ctrl_sig_t)dispatched_signal;
	// 数据 第3、4、5、6位
	dispatched_signal = (recieved_signal & 0x3C) >> 2;

	if (ctrl_signal == switch_on_off) {
		// recieved toggle status is 0x01(OFF) OR 0x02(ON) 
		if (check_toggle_status(dispatched_signal)) {
			toggle_status = dispatched_signal - 1;
			ctrl_toggle_status_response(prev_toggle_status);
		}
		else if(dispatched_signal == 0x05)
		{
			display_mode_set = (disp_t)0;
			ctrl_display_mode_response(prev_display_mode_set);
		}
		else if(dispatched_signal == 0x0A)
		{
			display_mode_set = (disp_t)14;
			ctrl_display_mode_response(prev_display_mode_set);
		}
	} else if (ctrl_signal == switch_display_mode) {
		// Check
		if (check_display_mode_type(dispatched_signal)) {
			display_mode_set = (disp_t)dispatched_signal;
			ctrl_display_mode_response(prev_display_mode_set);
		}
	} else if (ctrl_signal == adjust_lumins_level) {
		// recieved lumins level start from 0x01
		if (check_lumins_level(dispatched_signal))
			lumins_level_set = dispatched_signal - 1;
			// lumins_level responsed each impluse dected
	} else if (ctrl_signal == adjust_speed_level) {
		// recieved speed level start from 0x01
		if (check_speed_level(dispatched_signal))
			speed_level = dispatched_signal - 1;
			// speed_level responsed each cycle(5ms)

	} else {
		// Error Ctrl & Do nothing
		_nop_();
	}
	uart_enable();
}

void com_isr() interrupt 4 using 2		// serial com 
{
	if (RI) {
		INT_PROC |= UART_PROC;
		exint_disable(); 
		uart_buffer = SBUF;
		RI = 0;
		exint_enable();
	}
	if (TI) {
        TI = 0;                 
        busy = NO;        
    }

}
