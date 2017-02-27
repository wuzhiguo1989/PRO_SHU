#include "e2prom.h"
#include "intrins.h"
void DisableEEPROM(void)
{
    IAP_CONTR = 0;          //禁止ISP/IAP操作
    IAP_CMD   = 0;          //去除ISP/IAP命令
    IAP_TRIG  = 0;          //防止ISP/IAP命令误触发
    IAP_ADDRH = 0xff;       //清0地址高字节
    IAP_ADDRL = 0xff;       //清0地址低字节，指向非EEPROM区，防止误操作
}
u8 EEPROM_read(u16 EE_address)
{
    u8 dat;
    IAP_CONTR = ENABLE_IAP;
    IAP_CMD = CMD_READ;
    IAP_ADDRL = EE_address;
    IAP_ADDRH = EE_address >> 8;
    IAP_TRIG = 0x5a;
    IAP_TRIG = 0xa5;
    _nop_();
    dat = IAP_DATA;
    DisableEEPROM();
    return dat;
}

void EEPROM_SectorErase(u16 EE_address)
{
    IAP_CONTR = ENABLE_IAP;  //设置等待时间，允许ISP/IAP操作，送一次就够
    IAP_CMD = CMD_ERASE;     //送字节写命令，命令不需改变时，不需重新送命令
    IAP_ADDRL = EE_address;
    IAP_ADDRH = EE_address >> 8;
    IAP_TRIG = 0x5a;
    IAP_TRIG = 0xa5;
    _nop_();
    DisableEEPROM();
}

void EEPROM_write(u16 EE_address,u8 dat)
{

    IAP_CONTR = ENABLE_IAP;  //设置等待时间，允许ISP/IAP操作，送一次就够
    IAP_CMD = CMD_PROGRAM;   //送字节写命令，命令不需改变时，不需重新送命令
    IAP_ADDRL = EE_address;
    IAP_ADDRH = EE_address >> 8;
	IAP_DATA = dat;
    IAP_TRIG = 0x5a;
    IAP_TRIG = 0xa5;
    _nop_();
    DisableEEPROM();
}

