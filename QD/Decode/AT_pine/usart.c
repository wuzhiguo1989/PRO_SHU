#include "usart.h"
#include "string.h"

static const u32 code  BaudRateTimer = 65536UL - (MAIN_Fosc / 4) / 9600;
static bool share_timer2_init_finish = FALSE;
volatile  COMx_Define idata COM1;

void USART_Share_Timer2() {
    if (!share_timer2_init_finish) 
    {
        Timer2_Stop();              //Timer stop
        
        Timer2_AsTimer();           //Timer2 set As Timer
        Timer2_1T() ;               //Timer2 set as 1T mode
        // T2_Load(BaudRateTimer);
        T2L = 0x8F;     //设定定时初值
        T2H = 0xFD;     //设定定时初值
        Timer2_InterruptDisable();  //禁止中断
        
        Timer2_Run();               //Timer run enable

        share_timer2_init_finish = TRUE;
    }
}

void USART_Init(COMx_Define idata *com) {
    com->TX_read = 0;
    com->TX_write = 0;
    com->B_TX_busy = 0;
    com->RX_Cnt = 0;
    com->RX_TimeOut = 0;
    com->B_RX_OK = 0;
}

// Handle usart trigger: TimeOut(usart command gap) count down to zero
bool USART_RX_Block(COMx_Define idata *com) {
    if (!com->RX_Cnt) 
        return FALSE;

    if (com->RX_TimeOut && !--com->RX_TimeOut)
        com->B_RX_OK = TRUE;

    if (!com->RX_TimeOut)
        com->B_RX_OK = TRUE;

    return com->B_RX_OK;
}

void USART_Finish_Rx(COMx_Define idata *com) {
    com->RX_Cnt = 0;
    com->B_RX_OK = FALSE;
}