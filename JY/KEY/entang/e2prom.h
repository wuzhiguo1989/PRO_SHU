#ifndef __EEPROM_H_
#define __EEPROM_H_
#include "stc15wxxxx.h"
#define CMD_IDLE 0
#define CMD_READ 1
#define CMD_PROGRAM 2
#define CMD_ERASE 3
#define ENABLE_IAP 0x80


#define ADDRESS_FOR_PASSWORD 0x0000
#define ADDRESS_FOR_TIMESTAMP 0x0200

void DisableEEPROM(void);

u8 EEPROM_read(u16 EE_address);

void EEPROM_SectorErase(u16 EE_address);

void EEPROM_write(u16 EE_address,u8 dat);

#endif