#ifndef __EEPROM_H_
#define __EEPROM_H_
#include "stc15wxxxx.h"
#define CMD_IDLE 0
#define CMD_READ 1
#define CMD_PROGRAM 2
#define CMD_ERASE 3
#define ENABLE_IAP 0x80


#define ADDRESS_FOR_COUNTER_OPEN_KEY_CLIBE 0xEE00
#define ADDRESS_FOR_COUNTER_OPEN_FLOOD_DOOR 0xEE00

#define ADDRESS_FOR_A 0xF000
#define ADDRESS_FOR_B 0xF004

#define ADDRESS_FOR_COUNTER_READ_A_AND_B 0xEC00

#define ADDRESS_FOR_COUNTER_WRITE_A 0xF200
#define ADDRESS_FOR_COUNTER_CHANGE_BLE_NAME 0xF204

void DisableEEPROM(void);

u8 EEPROM_read(u16 EE_address);

void EEPROM_SectorErase(u16 EE_address);

void EEPROM_write(u16 EE_address,u8 dat);

#endif