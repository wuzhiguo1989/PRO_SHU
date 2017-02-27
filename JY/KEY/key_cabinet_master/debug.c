#include "debug.h"
#include "usart.h"
#include "stdio.h"

void debug_init() {
	S3_8bit();
	S3_BRT_UseTimer2();
    S3_USE_P00P01();
    
    // S3_Int_Enable();
    S3_Int_Disable();
    S3_RX_Enable();

    USART_Share_Timer2();

    SET_TI3();
}

// /*----------------------------
// UART3 中断服务程序
// -----------------------------*/
// void Uart3() interrupt UART3_VECTOR using 1
// {
//     if (S3CON & S3RI)
//     {
//         S3CON &= ~S3RI;         //清除S3RI位
//     }
//     if (S3CON & S3TI)
//     {
//         S3CON &= ~S3TI;         //清除S3TI位
//         busy = 0;               //清忙标志
//     }
// }

char putchar (char c)
{
	while (!TI3);  /* wait until transmitter ready */
	CLR_TI3();
	S3BUF = c;      /* output character */
	return (c);
}


//-----------------------------------------------
char _getkey (void)
{
	char c;

	while (!RI3);  /* wait for a character */
	c = S3BUF;
	CLR_RI3();       /* clear receive interrupt bit */

	return (c);
}


