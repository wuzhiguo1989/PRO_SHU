#include "e2prom.h"
#include "intrins.h"
#include "dwscreen.h"

u32 password_adminstrator = 0;
u32 password_guest = 0;
static void DisableEEPROM(void)
{
    IAP_CONTR = 0;          //禁止ISP/IAP操作
    IAP_CMD   = 0;          //去除ISP/IAP命令
    IAP_TRIG  = 0;          //防止ISP/IAP命令误触发
    IAP_ADDRH = 0xff;       //清0地址高字节
    IAP_ADDRL = 0xff;       //清0地址低字节，指向非EEPROM区，防止误操作
}
static u8 EEPROM_read(u16 EE_address)
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

static void EEPROM_SectorErase(u16 EE_address)
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

static void EEPROM_write(u16 EE_address,u8 dat)
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

void password_init()
{
    // 检查密码是否被修改过  
    if(EEPROM_read(ADDRESS_FOR_PASSWORD_MODIFY_FLAG) == 0xFF)
    {
    // 从INIT中取
         password_adminstrator =  EEPROM_read(ADDRESS_FOR_PASSWORD_ADMIN_INIT) << 24 | EEPROM_read(ADDRESS_FOR_PASSWORD_ADMIN_INIT + 1) << 16 | EEPROM_read(ADDRESS_FOR_PASSWORD_ADMIN_INIT + 2) << 8 | EEPROM_read(ADDRESS_FOR_PASSWORD_ADMIN_INIT + 3);
         password_guest = EEPROM_read(ADDRESS_FOR_PASSWORD_GUEST_INIT) << 24 | EEPROM_read(ADDRESS_FOR_PASSWORD_GUEST_INIT + 1) << 16 | EEPROM_read(ADDRESS_FOR_PASSWORD_GUEST_INIT + 2) << 8 | EEPROM_read(ADDRESS_FOR_PASSWORD_GUEST_INIT + 3);
    }
    else
    {
    // 从SET中取
         password_adminstrator =  EEPROM_read(ADDRESS_FOR_PASSWORD_ADMIN_SET) << 24 | EEPROM_read(ADDRESS_FOR_PASSWORD_ADMIN_SET + 1) << 16 | EEPROM_read(ADDRESS_FOR_PASSWORD_ADMIN_SET + 2) << 8 | EEPROM_read(ADDRESS_FOR_PASSWORD_ADMIN_SET + 3);
         password_guest = EEPROM_read(ADDRESS_FOR_PASSWORD_GUEST_SET) << 24 | EEPROM_read(ADDRESS_FOR_PASSWORD_GUEST_SET + 1) << 16 | EEPROM_read(ADDRESS_FOR_PASSWORD_GUEST_SET + 2) << 8 | EEPROM_read(ADDRESS_FOR_PASSWORD_GUEST_SET + 3);
    }
}

void store_all_password()
{
    EEPROM_SectorErase(ADDRESS_FOR_PASSWORD_MODIFY_FLAG);
    EEPROM_write(ADDRESS_FOR_PASSWORD_MODIFY_FLAG, 1);
    EEPROM_write(ADDRESS_FOR_PASSWORD_ADMIN_SET, password_adminstrator >> 24);
    EEPROM_write(ADDRESS_FOR_PASSWORD_ADMIN_SET + 1, password_adminstrator >> 16);
    EEPROM_write(ADDRESS_FOR_PASSWORD_ADMIN_SET + 2, password_adminstrator >> 8);
    EEPROM_write(ADDRESS_FOR_PASSWORD_ADMIN_SET + 3, password_adminstrator);
    EEPROM_write(ADDRESS_FOR_PASSWORD_GUEST_SET, password_guest >> 24);
    EEPROM_write(ADDRESS_FOR_PASSWORD_GUEST_SET + 1, password_guest >> 16);
    EEPROM_write(ADDRESS_FOR_PASSWORD_GUEST_SET + 2, password_guest >> 8);
    EEPROM_write(ADDRESS_FOR_PASSWORD_GUEST_SET + 3, password_guest);
}

void reset_all_password()
{
    EEPROM_SectorErase(ADDRESS_FOR_PASSWORD_MODIFY_FLAG);
    password_adminstrator =  EEPROM_read(ADDRESS_FOR_PASSWORD_ADMIN_INIT) << 24 | EEPROM_read(ADDRESS_FOR_PASSWORD_ADMIN_INIT + 1) << 16 | EEPROM_read(ADDRESS_FOR_PASSWORD_ADMIN_INIT + 2) << 8 | EEPROM_read(ADDRESS_FOR_PASSWORD_ADMIN_INIT + 3);
    password_guest = EEPROM_read(ADDRESS_FOR_PASSWORD_GUEST_INIT) << 24 | EEPROM_read(ADDRESS_FOR_PASSWORD_GUEST_INIT + 1) << 16 | EEPROM_read(ADDRESS_FOR_PASSWORD_GUEST_INIT + 2) << 8 | EEPROM_read(ADDRESS_FOR_PASSWORD_GUEST_INIT + 3);
}

void param_init()
{
    // 检查密码是否被修改过  
    if(EEPROM_read(ADDRESS_FOR_PARAM_MODIFY_FLAG) == 0xFF)
    {
    // 从INIT中取
        time_fan_delay_stop =  EEPROM_read(ADDRESS_FOR_PARAM_INIT);
        time_fan_delay_full = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 1);
        time_feed_suction = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 2);
        time_feed_discharge = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 3);
        time_feed_intercept = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 4);
        time_spray_wash = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 5);
        time_spray_wash_between = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 6);
        counter_for_spray_wash = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 7); 
        time_feed_lack = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 8);
        counter_for_lack_alter = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 9);
        tem_alter = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 10);
        tem_stop = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 11);
        time_feed_stop = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 12);
        feed_lack_signal = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 13);
        updata_data_from_dwscreen();
    }
    else
    {
    // 从SET中取
        time_fan_delay_stop =  EEPROM_read(ADDRESS_FOR_PARAM_SET + 1);
        time_fan_delay_full = EEPROM_read(ADDRESS_FOR_PARAM_SET + 2);
        time_feed_suction = EEPROM_read(ADDRESS_FOR_PARAM_SET + 3);
        time_feed_discharge = EEPROM_read(ADDRESS_FOR_PARAM_SET + 4);
        time_feed_intercept = EEPROM_read(ADDRESS_FOR_PARAM_SET + 5);
        time_spray_wash = EEPROM_read(ADDRESS_FOR_PARAM_SET + 6);
        time_spray_wash_between = EEPROM_read(ADDRESS_FOR_PARAM_SET + 7);
        counter_for_spray_wash = EEPROM_read(ADDRESS_FOR_PARAM_SET + 8); 
        time_feed_lack = EEPROM_read(ADDRESS_FOR_PARAM_SET + 9);
        counter_for_lack_alter = EEPROM_read(ADDRESS_FOR_PARAM_SET + 10);
        tem_alter = EEPROM_read(ADDRESS_FOR_PARAM_SET + 11);
        tem_stop = EEPROM_read(ADDRESS_FOR_PARAM_SET + 12);
        time_feed_stop = EEPROM_read(ADDRESS_FOR_PARAM_SET + 13);
        feed_lack_signal = EEPROM_read(ADDRESS_FOR_PARAM_SET + 14);
        updata_data_from_dwscreen();
    }    
}

void store_all_para()
{
    EEPROM_SectorErase(ADDRESS_FOR_PARAM_MODIFY_FLAG);
    EEPROM_write(ADDRESS_FOR_PARAM_MODIFY_FLAG, 1);
    // ADDRESS_FOR_PARAM_MODIFY_FLAG == ADDRESS_FOR_PARAM_SET
    EEPROM_write(ADDRESS_FOR_PARAM_SET + 1, time_fan_delay_stop);
    EEPROM_write(ADDRESS_FOR_PARAM_SET + 2, time_fan_delay_full);
    EEPROM_write(ADDRESS_FOR_PARAM_SET + 3, time_feed_suction);
    EEPROM_write(ADDRESS_FOR_PARAM_SET + 4, time_feed_discharge);
    EEPROM_write(ADDRESS_FOR_PARAM_SET + 5, time_feed_intercept);
    EEPROM_write(ADDRESS_FOR_PARAM_SET + 6, time_spray_wash);
    EEPROM_write(ADDRESS_FOR_PARAM_SET + 7, time_spray_wash_between);
    EEPROM_write(ADDRESS_FOR_PARAM_SET + 8, counter_for_spray_wash);
    EEPROM_write(ADDRESS_FOR_PARAM_SET + 9, time_feed_lack);
    EEPROM_write(ADDRESS_FOR_PARAM_SET + 10, counter_for_lack_alter);
    EEPROM_write(ADDRESS_FOR_PARAM_SET + 11, tem_alter);
    EEPROM_write(ADDRESS_FOR_PARAM_SET + 12, tem_stop);
    EEPROM_write(ADDRESS_FOR_PARAM_SET + 13, time_feed_stop);
    EEPROM_write(ADDRESS_FOR_PARAM_SET + 14, feed_lack_signal);
}

void reset_all_para()
{
    EEPROM_SectorErase(ADDRESS_FOR_PARAM_MODIFY_FLAG);
    time_fan_delay_stop =  EEPROM_read(ADDRESS_FOR_PARAM_INIT);
    time_fan_delay_full = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 1);
    time_feed_suction = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 2);
    time_feed_discharge = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 3);
    time_feed_intercept = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 4);
    time_spray_wash = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 5);
    time_spray_wash_between = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 6);
    counter_for_spray_wash = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 7); 
    time_feed_lack = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 8);
    counter_for_lack_alter = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 9);
    tem_alter = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 10);
    tem_stop = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 11);
    time_feed_stop = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 12);
    feed_lack_signal = EEPROM_read(ADDRESS_FOR_PARAM_INIT + 13);
}
