#include "ble.h"
#include "string.h"
#include "usart.h"
#include "task.h"
#include "config.h"
#include "stdio.h"
#include "share.h"
#include "sync.h"
#include "utils.h"

void Uart_Init() {
    USART_Init(&COM1);

    S1_8bit();
    S1_BRT_UseTimer2();     // S1 BRT Use Timer2;

    S1_Int_Enable();        // 允许中断

    S1_RX_Enable();         // Rx 接收允许
    S1_USE_P30P31();
    quasi_bidirectional(UART_TX_Mx, UART_TX_BIT);
    quasi_bidirectional(UART_RX_Mx, UART_RX_BIT);

    S1_TXD_RXD_OPEN();

    USART_Share_Timer2();
}

// Handle usart trigger: receiving "\r\n"
void UART1_int (void) interrupt UART1_VECTOR
{
    if (RI) {
        INT_PROC |= UART_DUTY;
        key_cabinet_number = SBUF;
        RI = 0;
    }
    if (TI) {
        TI = 0;                         
    }
}