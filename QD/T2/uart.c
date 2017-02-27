#include "uart.h"
#include "sync.h"
#include "def.h"
#include "display.h"
#include "shared.h"
#include "intrins.h"
#include "string.h"

static BOOL busy = NO;
#define     UART1_BUF_LENGTH    32


u8  TX1_Cnt;    //发送计数
u8  RX1_Cnt;    //接收计数
bit B_TX1_Busy = 0; //发送忙标志

u8  idata RX1_Buffer[UART1_BUF_LENGTH]; //接收缓冲
u8  idata TX1_Buffer[UART1_BUF_LENGTH]; //接收缓冲
//========================================================================
// 函数: void   init_uart()
// 描述: UART1初始化函数。 
//========================================================================
void init_uart()
{
	SCON = 0x50;                // 设置串口为8位可变波特率
    // T2L = 0x20;                 
    // T2H = 0xfe;                 // 9600 bps
    	T2L = 0xD8;		//设定定时初值
	T2H = 0xFF;		//设定定时初值
    AUXR |= 0x14;               // T2为1T模式, 并启动定时器2
    AUXR |= 0x01;               // 选择定时器2为串口1的波特率发生器
	PS = 1;						// 高优先级
	uart_enable();
    TX1_Cnt = 0;
    RX1_Cnt = 0;
}

static void SendData(u8 dat)
{
    while (B_TX1_Busy);                   //	等待前一个数据发送完成
    B_TX1_Busy = YES;                      
    SBUF = dat;                     //	发送当前数据
}

static void PrintString(u8 *puts)
{
    for (; *puts != 0;  puts++)     //遇到停止符0结束
    {
        // SBUF = *puts;
        // B_TX1_Busy = 1;
        // while(B_TX1_Busy);
            while (B_TX1_Busy);                   //	等待前一个数据发送完成
    B_TX1_Busy = YES;                      
    SBUF = *puts;                     //	发送当前数据
    }
}

void modify_name()
{
	 PrintString("AT+SCON+0040\r\n");
	// PrintString("AT+UART+4\r\n");
	//SendData(0x00);
}
//========================================================================
// 函数: void UART1_int (void) interrupt UART1_VECTOR
// 描述: UART1中断函数。
// 参数: nine.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================

void UART1_int (void) interrupt 4
{
    if(RI)
    {
        
				//exint_disable(); 
		RI = 0;
        RX1_Buffer[RX1_Cnt] = SBUF;
        if(++RX1_Cnt >= UART1_BUF_LENGTH)   RX1_Cnt = 0;    //防溢出
		if (SBUF == '\n')
		{	
			RX1_Cnt = 0;
			INT_PROC |= UART_PROC;
		}
	}

    if(TI)
    {
        TI = 0;
        B_TX1_Busy = 0;
    }
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
			display_mode_set = (disp_t)16;
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

void 
parse_crtl_signal_BLE( signal_t recieved_signal) {
	// local variable
	u8 dispatched_signal;
	ctrl_sig_t ctrl_signal;
	toggle_t prev_toggle_status = toggle_status;
	disp_t prev_display_mode_set = display_mode_set;

	//uart_disable();
	// 控制信号 第1和第2位 
	dispatched_signal = ((recieved_signal & 0xf0) >> 4) - 4;
	ctrl_signal = (ctrl_sig_t)dispatched_signal;
	// LOW 4 bit
	dispatched_signal = recieved_signal & 0x0f;

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
			display_mode_set = (disp_t)16;
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
	//uart_enable();
}

void parse_crtl_signal_from_BLE()
{
	u8 iterator,received[2];
	for (iterator = 0; iterator < 2; ++iterator)
	{
       	received[iterator] = RX1_Buffer[iterator];
		if(iterator == 1 && received[iterator] == '\n')
		{
			parse_crtl_signal_BLE(received[0]);
			break;
		}
	}
	uart_enable();
}
