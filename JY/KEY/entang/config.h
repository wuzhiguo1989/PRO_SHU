#ifndef     __CONFIG_H_
#define     __CONFIG_H_

/*********************************************************/

#include    "stc15wxxxx.h"


// MCU pin define
#define DRIVER  P03
#define BT_RTS  P12
#define BT_CTS  P13
#define MCU_CS	P14
#define BC      P34
#define CODE    P32

// MCU pin ble (TX <> RX)
#define BLE_TX_Mx	3
#define BLE_RX_Mx	3

// #define BLE_TX_BIT	0x01
// #define BLE_RX_BIT	0x02
#define BLE_TX_BIT	0x40
#define BLE_RX_BIT	0x80

//MCU pin driver
#define DRIVER_Mx  0
#define DRIVER_BIT 0x08


/***********************************************************/

#endif
