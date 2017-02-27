#ifndef __USART_H_
#define __USART_H_   

#include    "config.h"

typedef struct
{ 
    u8  TX_read;        //发送读指针
    u8  TX_write;       //发送写指针
    u8  B_TX_busy;      //忙标志
    u8  RX_Cnt;         //接收字节计数
    u8  RX_TimeOut;     //接收超时
    u8  B_RX_OK;        //接收块完成
} COMx_Define; 

extern volatile COMx_Define xdata COM1;
void USART_Share_Timer2();
void USART_Init(COMx_Define xdata *com);
#endif

