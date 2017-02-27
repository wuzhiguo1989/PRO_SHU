#include "bk8000l.h"
#include "string.h"
#include "usart.h"
#include "stdio.h"

#define bluetooth_send_command(x)   PrintString1(x)
#define bluetooth_recieve_data(x)   GetString1(x)

static volatile u8 idata TX1_Buffer[COM_TX1_Lenth];    //发送缓冲
static void TX1_write2buff(const u8 dat) 
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

void Bk8000l_Sync_Year(u8 param) {
    char cmd[11] = "AT+SY";
    u8 n = 5;
	  cmd[n++] = '0' + param/100;
	  cmd[n++] = param/10 - param/100*10 + '0';
	  cmd[n++] = param%10 + '0';
	  strcat(cmd,"\r\n");
    bluetooth_send_command(cmd);
}
void Bk8000l_Sync_Month(u8 param)
{
    
    char cmd[10] = "AT+AM";
    u8 n = 5;
    cmd[n++] = '0' + param/10;
    cmd[n++] = '0' + param%10;
    strcat(cmd,"\r\n");
    bluetooth_send_command(cmd);
}
void Bk8000l_Sync_Day(u8 param)
{
    char cmd[10] = "AT+SD";
    u8 n = 5;
    cmd[n++] = '0' + param/10;
    cmd[n++] = '0' + param%10;
    strcat(cmd,"\r\n");
    bluetooth_send_command(cmd); 
}
void Bk8000l_Sync_Hour(u8 param)
{
    char cmd[10] = "AT+SH";
    u8 n = 5;
    cmd[n++] = '0' + param/10;
    cmd[n++] = '0' + param%10;
    strcat(cmd,"\r\n");
    bluetooth_send_command(cmd); 
}
void Bk8000l_Sync_Minute(u8 param)
{
    char cmd[10] = "AT+BM";
    u8 n = 5;
    cmd[n++] = '0' + param/10;
    cmd[n++] = '0' + param%10;
    strcat(cmd,"\r\n");
    bluetooth_send_command(cmd); 
}

void Bk8000l_Sync_Time(u8 year_param, u8 month_param, u8 day_param, u8 hour_param,u8 minute_param)
{
    if(year_param > 199 || month_param > 12 || day_param > 31 || hour_param > 24 || minute_param > 60)
        return ;
		else{
					Bk8000l_Sync_Year(year_param);
					Bk8000l_Sync_Month(month_param);
					Bk8000l_Sync_Day(day_param);
					Bk8000l_Sync_Hour(hour_param);
					Bk8000l_Sync_Minute(minute_param);
		}
}

void Bk8000l_Change_DisplayMode(u8 param)
{
    char cmd[10] = "AT+SM";
    u8 n = 5;
    if(param > 10)
        return ;
    else{
        cmd[n++] = '0' + param/10;
        cmd[n++] = '0' + param%10;
        strcat(cmd,"\r\n");
        bluetooth_send_command(cmd);
    } 
}

void Bk8000l_Change_DisplayLimus(u8 param)
{
    char cmd[10] = "AT+SL";
    u8 n = 5;
    if(param > 15)
        return ;
    else{
        cmd[n++] = '0' + param/10;
        cmd[n++] = '0' + param%10;
        strcat(cmd,"\r\n");
        bluetooth_send_command(cmd);
    } 
}

void Bk8000l_Change_RedColor(u8 param)
{
    char cmd[11] = "AT+SR";
    u8 n = 5;
	cmd[n++] = '0' + param/100;
	cmd[n++] = '0' + param/10 - param/100*10;
	cmd[n++] = '0' + param%10;
	strcat(cmd,"\r\n");
    bluetooth_send_command(cmd);
}
void Bk8000l_Change_GreenColor(u8 param)
{
    char cmd[11] = "AT+SG";
    u8 n = 5;
    cmd[n++] = '0' + param/100;
    cmd[n++] = '0' + param/10 - param/100*10;
    cmd[n++] = '0' + param%10;
    strcat(cmd,"\r\n");
    bluetooth_send_command(cmd);
}
void Bk8000l_Change_BlueColor(u8 param)
{
    char cmd[11] = "AT+SB";
    u8 n = 5;
    cmd[n++] = '0' + param/100;
    cmd[n++] = '0' + param/10 - param/100*10;
    cmd[n++] = '0' + param%10;
    strcat(cmd,"\r\n");
    bluetooth_send_command(cmd);

}

void Bk8000l_Sync_Color(u8 red_param, u8 green_param, u8 blue_param)
{
    if(red_param > 255 || green_param > 255 || red_param > 255)
        return ;
    else
    {
        Bk8000l_Change_RedColor(red_param);
        Bk8000l_Change_GreenColor(green_param);
        Bk8000l_Change_BlueColor(blue_param);        
    }
}

void Bk8000l_Sync_ALARM1_Hour(u8 param)
{
    char cmd[12] = "AT+A1SH";
    u8 n = 7;
    cmd[n++] = '0' + param/10;
    cmd[n++] = '0' + param%10;
    strcat(cmd,"\r\n");
    bluetooth_send_command(cmd); 
}
void Bk8000l_Sync_ALARM1_Minute(u8 param)
{
    char cmd[12] = "AT+A1BM";
    u8 n = 7;
    cmd[n++] = '0' + param/10;
    cmd[n++] = '0' + param%10;
    strcat(cmd,"\r\n");
    bluetooth_send_command(cmd); 
}

void Bk8000l_Sync_ALARM1(u8 hour_param,u8 minute_param)
{
    if(hour_param > 24 || minute_param > 60)
        return ;
		else{
					Bk8000l_Sync_ALARM1_Hour(hour_param);
					Bk8000l_Sync_ALARM1_Minute(minute_param);
		}
}

void Bk8000l_Sync_ALARM2_Hour(u8 param)
{
    char cmd[12] = "AT+A2SH";
    u8 n = 7;
    cmd[n++] = '0' + param/10;
    cmd[n++] = '0' + param%10;
    strcat(cmd,"\r\n");
    bluetooth_send_command(cmd); 
}
void Bk8000l_Sync_ALARM2_Minute(u8 param)
{
    char cmd[12] = "AT+A2BM";
    u8 n = 7;
    cmd[n++] = '0' + param/10;
    cmd[n++] = '0' + param%10;
    strcat(cmd,"\r\n");
    bluetooth_send_command(cmd); 
}

void Bk8000l_Sync_ALARM2(u8 hour_param,u8 minute_param)
{
    if(hour_param > 24 || minute_param > 60)
        return ;
		else{
					Bk8000l_Sync_ALARM2_Hour(hour_param);
					Bk8000l_Sync_ALARM2_Minute(minute_param);
		}
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
