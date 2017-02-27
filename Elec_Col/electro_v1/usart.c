#include "usart.h"
#include "string.h"
#define BaudRateTimer 0xFD8F

static bool share_timer2_init_finish = FALSE;

volatile COMx_Define xdata COM2;
void USART_Share_Timer2() {
    if (!share_timer2_init_finish) {
        Timer2_Stop();              //Timer stop
        
        Timer2_AsTimer();           //Timer2 set As Timer
        Timer2_1T() ;               //Timer2 set as 1T mode
        T2_Load(BaudRateTimer);
        Timer2_InterruptDisable();  //禁止中断
        
        Timer2_Run();               //Timer run enable

        share_timer2_init_finish = TRUE;
    }
}

void USART_Init(COMx_Define xdata *com)
{
    com->TX_read = 0;
    com->TX_write = 0;
    com->B_TX_busy = 0;
    com->RX_Cnt = 0;
    com->RX_TimeOut = 0;
    com->B_RX_OK = 0;
}
