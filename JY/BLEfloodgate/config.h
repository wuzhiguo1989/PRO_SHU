#ifndef     __CONFIG_H_
#define     __CONFIG_H_

/*********************************************************/

#include    "stc15wxxxx.h"


// MCU pin define
#define DRIVER  P55
#define BT_RTS  P10
#define BT_CTS  P33
#define MCU_CS	P54

// MCU pin ble (TX <> RX)
#define BLE_TX_Mx	3
#define BLE_RX_Mx	3

#define BLE_TX_BIT	0x01
#define BLE_RX_BIT	0x02

//MCU pin driver
#define DRIVER_Mx  5
#define DRIVER_BIT 0x20


/***********************************************************/

#endif
