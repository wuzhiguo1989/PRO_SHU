#include "config.h"
#include "uart.h"

char putchar (char c)
{
	return TX_write2buff(COM[DEBUG_UART_ID], c);
}
