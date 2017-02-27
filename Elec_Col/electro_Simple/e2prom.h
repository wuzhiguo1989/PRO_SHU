#ifndef __EEPROM_H_
#define __EEPROM_H_
#include "stc15wxxxx.h"
#define CMD_IDLE 0
#define CMD_READ 1
#define CMD_PROGRAM 2
#define CMD_ERASE 3
#define ENABLE_IAP 0x80

#define ADDRESS_FOR_MILEAGE       0x0000
#define ADDRESS_FOR_MILEAGE_TOTAL 0x0200


u16 Read_mileage();
u32 Read_mileage_total();
void Store_mileage(u16 counter);
void Store_mileage_total(u32 mileage_total);
#endif