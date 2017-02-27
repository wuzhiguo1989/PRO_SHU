#ifndef     __CONFIG_H_
#define     __CONFIG_H_

/*********************************************************/

#include    "stc15wxxxx.h"

// MCU TEST
#define JC P34

// MCU pin ble (TX <> RX)
#define BT_RTS  P10
#define BT_CTS  P33
#define MCU_CS	P54


#define BLE_TX_Mx	3
#define BLE_RX_Mx	3

#define BLE_TX_BIT	0x80
#define BLE_RX_BIT	0x40

// MCU DRIVE define
// #define DRIVER_D1  P52
// #define DRIVER_D2  P04
// #define DRIVER_D3  P03
// #define DRIVER_D4  P02
// #define DRIVER_D5  P01
// #define DRIVER_D6  P00
// #define DRIVER_D7  P46
// #define DRIVER_D8  P45
// #define DRIVER_D9  P77
// #define DRIVER_D10  P76
// #define DRIVER_D11  P75
// #define DRIVER_D12  P74
// #define DRIVER_D13  P27
// #define DRIVER_D14  P26
// #define DRIVER_D15  P25
// #define DRIVER_D16  P24
// #define DRIVER_D17  P23
// #define DRIVER_D18  P22
// #define DRIVER_D19  P21
// #define DRIVER_D20  P20
// #define DRIVER_D21  P44
// #define DRIVER_D22  P43
// #define DRIVER_D23  P42
// #define DRIVER_D24  P41
// #define DRIVER_D25  P73
// #define DRIVER_D26  P72
// #define DRIVER_D27  P71
// #define DRIVER_D28  P70
#define DRIVER_D29  P40
#define DRIVER_D30  P64
#define high_pin(x) DRIVER_D##x = 1
#define low_pin(x) DRIVER_D##x = 0
void driver_io_init();
void driver_high_pin(u8 pin);
void driver_low_pin(u8 pin);
/***********************************************************/

#endif
