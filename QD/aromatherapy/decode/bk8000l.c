#include "bk8000l.h"
#include "string.h"
#include "usart.h"
#include "stdio.h"

#define bluetooth_send_command(x)   PrintString1(x)
#define bluetooth_recieve_data(x)   GetString1(x)

static volatile u8 idata TX1_Buffer[COM_TX1_Lenth];    //发送缓冲

void Delay100ms()		//@24.000MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 10;
	j = 31;
	k = 147;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

void TX1_write2buff(const u8 dat) 
{
    TX1_Buffer[COM1.TX_write] = dat;        // 装发送缓冲
    if(++COM1.TX_write >= COM_TX1_Lenth)    
        COM1.TX_write = 0;

    if(COM1.B_TX_busy == 0)                 // 空闲
    {  
        COM1.B_TX_busy = 1;                 // 标志忙
        TI = 1;                             // 触发发送中断
    }
}


static void PrintString1(u8 *puts)
{
    for (; *puts != 0;  puts++)  
        TX1_write2buff(*puts);     //遇到停止符0结束

}

void Bk8000l_Init() {

    memset(TX1_Buffer, 0, COM_TX1_Lenth);
    USART_Init(&COM1);

    S1_8bit();
    S1_BRT_UseTimer2();     // S1 BRT Use Timer2;

    S1_Int_Enable();        // 允许中断
    S1_RX_Enable();         // Rx 接收允许
    S1_USE_P30P31();
    S1_TXD_RXD_OPEN();

    USART_Share_Timer2();
}

void Bk8000l_Change_DisplayMode(u8 param)
{
    char cmd[10] = "A";
    u8 n = 1;
    if(param > 10)
        return ;
    else{
        cmd[n++] = '0' + param/10;
        cmd[n++] = '0' + param%10;
        bluetooth_send_command(cmd);
    } 
}

void Bk8000l_Change_Power(u8 param)
{
    char cmd[10] = "B";
    u8 n = 1;
    if(param > 4)
        return ;
    else{
        cmd[n++] = '0' + param%10;
        bluetooth_send_command(cmd);
    } 
}

void Bk8000l_Change_DisplayLimus(u8 param)
{
    char cmd[4] = "E";
    u8 n = 1;
    if(param > 15)
        return ;
    else{
        cmd[n++] = '0' + param/10;
        cmd[n++] = '0' + param%10;
        bluetooth_send_command(cmd);
    } 
}

void Bk8000l_Change_DisplaySpeed(u8 param)
{
    char cmd[4] = "F";
    u8 n = 1;
    if(param > 15)
        return ;
    else{
        cmd[n++] = '0' + param/10;
        cmd[n++] = '0' + param%10;
        bluetooth_send_command(cmd);
    } 

}


void Bk8000l_Sync_Color(u8 red_param, u8 green_param, u8 blue_param)
{
    char cmd[11] = "C";
    u8 n = 1;

    
    if(red_param > 255 || green_param > 255 || red_param > 255)
        return ;
    else
    {
        cmd[n++] = '0' + red_param/100;
        cmd[n++] = '0' + red_param/10 - red_param/100*10;
        cmd[n++] = '0' + red_param%10;
        cmd[n++] = '0' + green_param/100;
        cmd[n++] = '0' + green_param/10 - green_param/100*10;
        cmd[n++] = '0' + green_param%10;
        cmd[n++] = '0' + blue_param/100;
        cmd[n++] = '0' + blue_param/10 - blue_param/100*10;
        cmd[n++] = '0' + blue_param%10;
        bluetooth_send_command(cmd);
    }
}


void Bk8000l_Control_ALARM(u8 param)
{
    char cmd[3] = "D";
    u8 n = 1;
    cmd[n++] = '0' + param;
    if(param > 4)
        return ;
    else
        bluetooth_send_command(cmd);
}


void UART1_int (void) interrupt UART1_VECTOR
{
    if(RI)
    {
        RI = 0;
    }

    if(TI)
    {
        TI = 0;
        if(COM1.TX_read != COM1.TX_write)
        {
            SBUF = TX1_Buffer[COM1.TX_read];
            if(++COM1.TX_read >= COM_TX1_Lenth)     
                COM1.TX_read = 0;
        }
        else    
            COM1.B_TX_busy = 0;
    }
}
