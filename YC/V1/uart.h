#ifndef __UART_H
#define __UART_H	 

#include "config.h"
#include "typealias.h"

typedef struct
{ 
	u8	id;				//串口号
	u8	TX_read;		//发送读指针
	u8	TX_write;		//发送写指针
	u8	B_TX_busy;		//忙标志
	u8 	RX_Cnt;			//接收字节计数
	u8	RX_TimeOut;		//接收超时
	u8	B_RX_OK;		//接收块完成
} COMx_context; 

#define COM1_ID			0
#define COM2_ID			1

#define	COM_TX1_Length	32
#define	COM_RX1_Length	32
#define	COM_TX2_Length	256
#define	COM_RX2_Length	128


#define	TimeOutSet1		25
#define	TimeOutSet2		50

extern COMx_context xdata COM1, COM2;
extern COMx_context xdata *COM[MAX_UART_SUPPORT];

extern char xdata TX1_Buffer[COM_TX1_Length];	
extern char xdata RX1_Buffer[COM_RX1_Length];	
extern char xdata TX2_Buffer[COM_TX2_Length];	
extern char xdata RX2_Buffer[COM_RX2_Length];		

void uart_init();
bool UART_RX_Block(COMx_context xdata *);
void UART_Finish_Rx(COMx_context xdata *);
char TX_write2buff(COMx_context xdata *, char c);
void TX_str2buff(COMx_context xdata *, char *, int len);

#define TX_FW_ECHO
// #undef	TX_FW_ECHO	
#define TX_COMPENSATE_CRLF
// #undef TX_COMPENSATE_CRLF
void TX_forward(COMx_context xdata *target);

#endif