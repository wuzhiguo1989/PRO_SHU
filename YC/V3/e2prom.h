#ifndef __EEPROM_H_
#define __EEPROM_H_
#include "stc15w4k32s4.h"
#include "typealias.h"
#define CMD_IDLE 0
#define CMD_READ 1
#define CMD_PROGRAM 2
#define CMD_ERASE 3
#define ENABLE_IAP 0x80

#define ADDRESS_FOR_PASSWORD_ADMIN_INIT 0x4000
#define ADDRESS_FOR_PASSWORD_GUEST_INIT 0x4004

#define ADDRESS_FOR_PASSWORD_MODIFY_FLAG  0x4200
#define ADDRESS_FOR_PASSWORD_ADMIN_SET  0x4200
#define ADDRESS_FOR_PASSWORD_GUEST_SET  0x4205

#define ADDRESS_FOR_PARAM_MODIFY_FLAG  0x4400
#define ADDRESS_FOR_PARAM_SET     0x4401

#define ADDRESS_FOR_PARAM_INIT    0x4600


extern u32 password_adminstrator;
extern u32 password_guest;

void password_init();

void param_init();

void store_all_password();

void reset_all_password();


void store_all_para();

void reset_all_para();


#endif