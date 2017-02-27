#ifndef     __CONFIG_H_
#define     __CONFIG_H_

/*********************************************************/

#include    "stc15wxxxx.h"

// MCU pin ble (TX <> RX)
#define UART_TX_Mx	3
#define UART_RX_Mx	3

#define UART_TX_BIT	0x01
#define UART_RX_BIT	0x02

// MCU DRIVE define
#define DRIVER_D1  P17
#define DRIVER_D2  P16
#define DRIVER_D3  P15
#define DRIVER_D4  P14
#define DRIVER_D5  P13
#define DRIVER_D6  P12
#define DRIVER_D7  P11
#define DRIVER_D8  P10
#define DRIVER_D9  P03
#define DRIVER_D10  P02
#define DRIVER_D11  P01
#define DRIVER_D12  P00
#define DRIVER_D13  P27
#define DRIVER_D14  P26
#define DRIVER_D15  P25
#define DRIVER_D16  P24
#define DRIVER_D17  P23
#define DRIVER_D18  P22
#define DRIVER_D19  P21
#define DRIVER_D20  P20
#define DRIVER_D21  P37
#define DRIVER_D22  P36
#define DRIVER_D23  P35
#define DRIVER_D24  P34
#define DRIVER_D25  P33
#define DRIVER_D26  P32
#define DRIVER_D27  P55
#define DRIVER_D28  P54
#define high_pin(x) DRIVER_D##x = 1
#define low_pin(x) DRIVER_D##x = 0
void driver_io_init();
void driver_high_pin(u8 pin);
void driver_low_pin(u8 pin);
/***********************************************************/

#endif
