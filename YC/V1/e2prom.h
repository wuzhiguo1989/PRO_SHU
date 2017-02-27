#ifndef __EEPROM_H_
#define __EEPROM_H_
#include "stc15w4k32s4.h"
#include "typealias.h"
#define CMD_IDLE 0
#define CMD_READ 1
#define CMD_PROGRAM 2
#define CMD_ERASE 3
#define ENABLE_IAP 0x80

#define ADDRESS_FOR_VAILABLE_FLAG  0x3E00
#define ADDRESS_FOR_VAILABLE_SET  0x3E01

#define ADDRESS_FOR_PASSWORD_ADMIN_INIT 0x4000
#define ADDRESS_FOR_PASSWORD_GUEST_INIT 0x4004
#define ADDRESS_FOR_PASSWORD_SUPER_INIT 0x4008

#define ADDRESS_FOR_PASSWORD_MODIFY_FLAG  0x4200
#define ADDRESS_FOR_PASSWORD_ADMIN_SET  0x4201
#define ADDRESS_FOR_PASSWORD_GUEST_SET  0x4205

#define ADDRESS_FOR_PARAM_MODIFY_FLAG  0x4400
#define ADDRESS_FOR_PARAM_SET     0x4401

#define ADDRESS_FOR_PARAM_INIT    0x4600


extern u32 password_adminstrator;
extern u32 password_guest;
extern u32 password_super;

void password_init();

void param_init();

void store_all_password();

void reset_all_password();


void store_all_para();

void reset_all_para();

void reset_available_set();
void store_available_set(u8 *time_start);
void available_system_set();


#endif