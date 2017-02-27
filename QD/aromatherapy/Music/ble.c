#include "ble.h"
#include "string.h"
#include "usart.h"
#include "task.h"
#include "config.h"
#include "stdio.h"
#include "sync.h"
#include "utils.h"
#include "key.h"
#include "power.h"
#include "music.h"
#include "display.h"


#define ble_send_command(x)   PrintString1(x)
#define ble_recieve_data(x)   GetString1(x)
static volatile u8 xdata TX1_Buffer[COM_TX1_Lenth];    //发送缓冲
static volatile u8 xdata RX1_Buffer[COM_RX1_Lenth];    //接收缓冲

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



static void GetString1(u8 *received) {
    if (COM1.RX_Cnt) 
        memcpy(received, RX1_Buffer, COM1.RX_Cnt);
    received[COM1.RX_Cnt] = '\0';
    memset(RX1_Buffer, 0, COM_RX1_Lenth);
    COM1.RX_Cnt = 0;
    COM1.B_RX_OK = FALSE;
}

static void PrintString1(u8 *puts)
{
    for (; *puts != 0;  puts++)  TX1_write2buff(*puts);     //遇到停止符0结束
}

void BLE_Init() {

    memset(TX1_Buffer, 0, COM_TX1_Lenth);
    memset(RX1_Buffer, 0, COM_RX1_Lenth);
    USART_Init(&COM1);

    S1_8bit();
    S1_BRT_UseTimer2();     // S1 BRT Use Timer2;

    S1_Int_Enable();        // 允许中断

    S1_RX_Enable();         // Rx 接收允许
    S1_USE_P36P37();
    quasi_bidirectional(BLE_TX_Mx, BLE_TX_BIT);
    quasi_bidirectional(BLE_RX_Mx, BLE_RX_BIT);

    S1_TXD_RXD_OPEN();

    USART_Share_Timer2();
}

#define GP228_CMD_SIZE   4

static void uart_dispatch_gp228_cmd(const char xdata *cmd) {
    // GP228+MS=<1-4>  香薰功率
    // GP228+MT=<0,1>  0 香薰关闭 1香薰打开
    // GP228+SS=<1-8>  8首歌
    // GP228+SV=<0-15> 16级音量
    // GP228+ST=<0,1>  0 音乐关闭 1音乐打开
    // GP228+LS=<R...G...B...>
    // GP228+LT=<0,1>  灯光打开、关闭
    if (memcmp(cmd, "+MS=", GP228_CMD_SIZE) == 0) {
            power_select_level(cmd[4] - '0');
    } else if (memcmp(cmd, "+MT=", GP228_CMD_SIZE) == 0) {
            power_switch_on_off(cmd[4] - '0');
    } else if (memcmp(cmd, "+SS=", GP228_CMD_SIZE) == 0){
            music_select_songs(cmd[4] - '0');
    } else if (memcmp(cmd, "+SV=", GP228_CMD_SIZE) == 0) {
            music_select_volume(cmd);
    } else if (memcmp(cmd, "+ST=", GP228_CMD_SIZE) == 0) {
            music_switch_on_off(cmd[4] - '0');
    } else if (memcmp(cmd, "+LS=", GP228_CMD_SIZE) == 0) {
            led_syc_color(cmd);
    } else if (memcmp(cmd, "+LT=", GP228_CMD_SIZE) == 0) {
            led_switch_on_off(cmd[4] - '0');
    } 
 }

void uart_process_recieved() {
    char xdata *gp228_param;
    char received[COM_RX1_Lenth];
    ble_recieve_data(received);
    gp228_param = strchr(received, '+');
    if (gp228_param) {
        uart_dispatch_gp228_cmd(gp228_param);
    }
}

// Handle usart trigger: receiving "\r\n"
void UART1_int (void) interrupt UART1_VECTOR
{
    if(RI)
    {
        RI = 0;

        if(COM1.B_RX_OK == 0)
        {
            if (COM1.RX_Cnt >= COM_RX1_Lenth)
                COM1.RX_Cnt = 0;
            RX1_Buffer[COM1.RX_Cnt++] = SBUF;
            COM1.RX_TimeOut = TimeOutSet1;
        }

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
