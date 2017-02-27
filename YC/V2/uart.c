#include "uart.h"
#include "task.h"
#include <string.h>
#include <ctype.h>

COMx_context xdata COM1, COM2;
COMx_context xdata *COM[MAX_UART_SUPPORT];

char xdata TX1_Buffer[COM_TX1_Length];	
char xdata RX1_Buffer[COM_RX1_Length];	
char xdata TX2_Buffer[COM_TX2_Length];	
char xdata RX2_Buffer[COM_RX2_Length];	 
	

static char TX1_write2buff(char dat);
static char TX2_write2buff(char dat);

static void context_init(COMx_context xdata *c) {
	c->TX_read = 0;
	c->TX_write = 0;
	c->B_TX_busy = 0;
	c->RX_Cnt = 0;
	c->RX_TimeOut = 0;
	c->B_RX_OK = 0;
}

static bool share_timer2_init_once_token = FALSE;

// 115200的波特率
static void uart_share_timer2() {
	if (!share_timer2_init_once_token) {
		Timer2_Stop();              //Timer stop
	    Timer2_AsTimer();           //Timer2 set As Timer
	    Timer2_1T() ;               //Timer2 set as 1T mode
	    Timer2_Load(65536ul - (MAIN_Fosc / 4) / 115200ul);
	    Timer2_InterruptDisable();  //禁止中断    
	    Timer2_Run();               //Timer run enable
	    share_timer2_init_once_token = TRUE;
	}
}

void uart_init() {

	// COM1
	S1_8bit();
	S1_BRT_UseTimer2();
    S1_USE_P30P31();
    
    S1_Int_Enable();
    S1_RX_Enable();
    COM1.id = COM1_ID;
    memset(TX1_Buffer, 0, COM_TX1_Length);
    memset(RX1_Buffer, 0, COM_RX1_Length);
    context_init(&COM1);
    COM[COM1_ID] = &COM1;
    
    // COM2
	S2_8bit();
    S2_USE_P10P11();
    S2_MODE0();
	S2_RX_Enable();
    S2_Int_Enable();    
    SET_TI2();
    COM2.id = COM2_ID;
    memset(TX2_Buffer, 0, COM_TX2_Length);
    memset(RX2_Buffer, 0, COM_RX2_Length);
    context_init(&COM2);
    COM[COM2_ID] = &COM2;
	uart_share_timer2();
}

// Handle usart trigger: TimeOut(usart command gap) count down to zero
bool UART_RX_Block(COMx_context xdata *context) {
    if (!context->RX_Cnt) 									// no data recieved
        return false;

    if (context->RX_TimeOut && !--context->RX_TimeOut)		
        context->B_RX_OK = true;

    if (!context->RX_TimeOut)
        context->B_RX_OK = true;

    return context->B_RX_OK;
}

void UART_Finish_Rx(COMx_context xdata *context) {
    context->RX_Cnt = 0;
    context->B_RX_OK = false;
}

// Forward your UART TX -> target RX
void TX_forward(COMx_context xdata *target) {
	u8 i;

	for (i=0; i<COM1.RX_Cnt; i++) {
		TX_write2buff(target, RX1_Buffer[i]);
#ifdef TX_FW_ECHO
		TX_write2buff(&COM1, RX1_Buffer[i]);
#endif
	}
		
#ifdef TX_COMPENSATE_CRLF
	if (RX1_Buffer[COM1.RX_Cnt - 1] != '\n') {
		TX_write2buff(target, '\r');
		TX_write2buff(target, '\n');
		TX_write2buff(&COM1, '\r');
		TX_write2buff(&COM1, '\n');
	}
#endif
}

char TX_write2buff(COMx_context xdata *context, char c) {
	if(context->id == COM1_ID)	return TX1_write2buff(c);
	if(context->id == COM2_ID)	return TX2_write2buff(c);
	return c;
}

void TX_str2buff(COMx_context xdata *context, char *str, int len) {
	int i;
#ifdef DEBUG_MSG
	TX_write2buff(COM[DEBUG_UART_ID], context->id + '1');
	TX_write2buff(COM[DEBUG_UART_ID], ':');
#endif	
	for (i=0; i<len; ++i) {
		TX_write2buff(context, str[i]);
#ifdef DEBUG_MSG
		TX_write2buff(COM[DEBUG_UART_ID], str[i]);
#endif	
	}
}

static char TX1_write2buff(char c)	
{
	TX1_Buffer[COM1.TX_write] = c;	
	if(++COM1.TX_write >= COM_TX1_Length)	
		COM1.TX_write = 0;

	if(COM1.B_TX_busy == 0)		
	{  
		COM1.B_TX_busy = 1;		
		SET_TI1();					
	}
	return c;
}

static char TX2_write2buff(char c)	
{
	TX2_Buffer[COM2.TX_write] = c;	
	if(++COM2.TX_write >= COM_TX2_Length)	
		COM2.TX_write = 0;

	if(COM2.B_TX_busy == 0)		
	{  
		COM2.B_TX_busy = 1;		
		SET_TI2();					
	}
	return c;
}



void UART1_int (void) interrupt UART1_VECTOR using 2
{
	if(RI)
	{
		CLR_RI1();
		if(COM1.B_RX_OK == 0)
		{
			if(COM1.RX_Cnt >= COM_RX1_Length)	COM1.RX_Cnt = 0;
			RX1_Buffer[COM1.RX_Cnt++] = SBUF;
			COM1.RX_TimeOut = TimeOutSet1;
		}
	}

	if(TI)
	{
		CLR_TI1();
		if(COM1.TX_read != COM1.TX_write)
		{
		 	SBUF = TX1_Buffer[COM1.TX_read];
			if(++COM1.TX_read >= COM_TX1_Length)		COM1.TX_read = 0;
		}
		else	COM1.B_TX_busy = 0;
	}
}


void UART2_int (void) interrupt UART2_VECTOR
{
    if((S2CON & 1) != 0)
    {
		if(COM2.B_RX_OK == 0)
		{
			if(COM2.RX_Cnt >= COM_RX2_Length)
				COM2.RX_Cnt = 0;
			RX2_Buffer[COM2.RX_Cnt++] = S2BUF;
			COM2.RX_TimeOut = TimeOutSet2;
		}
        S2CON &= ~1;    //Clear Rx flag
    }

    if((S2CON & 2) != 0)
    {
        S2CON &= ~2;    //Clear Tx flag
		if(COM2.TX_read != COM2.TX_write)
		{
		 	S2BUF = TX2_Buffer[COM2.TX_read];
			if(++COM2.TX_read >= COM_TX2_Length)		COM2.TX_read = 0;
		}
		else	COM2.B_TX_busy = 0;
    }
}


