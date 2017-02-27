#include "entang.h"
#include "ble.h"
#include "i2c.h"
#include "utils.h"
#include "stc15wxxxx.h"

/***********************************************************************************
 * Function
 ***********************************************************************************/
void Delay1ms()     //@24.000MHz
{
    unsigned char i, j;

    i = 24;
    j = 85;
    do
    {
        while (--j);
    } while (--i);
}


void ENTANG_A_delay_ms(unsigned char n)
{
    unsigned char i;
    for(i = 0; i < n; i++)
        Delay1ms();
}


unsigned char ENTANG_A_init(unsigned char *ENTANG_A_user_key_password, unsigned char *key_in)
{
    unsigned char error_code;
    unsigned char tx_data[16];
    unsigned char sub_addr;

    // ENTANG-A Chip Software Reset
    tx_data[0] = 0x01;
    error_code = _i2c_write(DEV_ADDR, CMD_RESET, tx_data, 1);
    ENTANG_A_delay_ms(20);

    //For Final Key Generation
    sub_addr = CMD_USER_KEY_SET;
    error_code = _i2c_write(DEV_ADDR, sub_addr, ENTANG_A_user_key_password, 4);
    if (error_code) 
        return error_code;

    tx_data[0] = 0x00;
    error_code = _i2c_write(DEV_ADDR, CMD_MODE0, tx_data, 1);
    if ( error_code ) 
        return error_code;
    ENTANG_A_delay_ms(1);

    // ENTANG-A Ready
    tx_data[0] = 0x08;
    error_code = _i2c_write(DEV_ADDR, CMD_MODE1, tx_data, 1);
    if ( error_code ) 
        return error_code;
    ENTANG_A_delay_ms(5);


    error_code = _i2c_read (DEV_ADDR, CMD_AES_ENC, key_in, 16);
    return ERROR_CODE_TRUE;
}


void ENTANG_key_init()
{
    u8 user_password[4], key_init[16];
    Read_Password(user_password, 0, 3);
    ENTANG_A_init(user_password, key_init);
}


unsigned char ENTANG_A_enc(unsigned char *pt, unsigned char *ct)
{
    unsigned char error_code;
    unsigned char tx_data[16];
    unsigned char sub_addr;

    sub_addr = CMD_MODE0;
    tx_data[0] = 0x00;
    error_code = _i2c_write(DEV_ADDR, sub_addr, tx_data, 1);

    tx_data[0] = 0x08;
    error_code = _i2c_write(DEV_ADDR, CMD_MODE1, tx_data, 1);
    if ( error_code ) 
        return error_code;
    ENTANG_A_delay_ms(5);

    // Operating AES128 Engine - ENTANG
    sub_addr = CMD_AES_ENC;   
    error_code = _i2c_write(DEV_ADDR, sub_addr, pt, 16);
    if ( error_code ) 
        return error_code;
    ENTANG_A_delay_ms(5);

    error_code = _i2c_read (DEV_ADDR, sub_addr, ct, 16);
    if ( error_code ) 
        return error_code;
    return ERROR_CODE_TRUE;

}


unsigned char ENTANG_A_dec(unsigned char *pt, unsigned char *ct)
{
    unsigned char error_code;
    unsigned char tx_data[16];
    unsigned char sub_addr;

    sub_addr = CMD_MODE0;
    tx_data[0] = 0x04;  // Decryption Mode Enable
    error_code = _i2c_write(DEV_ADDR, sub_addr, tx_data, 1);

    tx_data[0] = 0x08;
    error_code = _i2c_write(DEV_ADDR, CMD_MODE1, tx_data, 1);
    if ( error_code ) 
        return error_code;
    ENTANG_A_delay_ms(5);


    sub_addr = CMD_AES_DEC;
    error_code = _i2c_write(DEV_ADDR, sub_addr, pt, 16);
    if (error_code) 
        return error_code;
    ENTANG_A_delay_ms(5);

    error_code = _i2c_read (DEV_ADDR, sub_addr, ct, 16);
    if ( error_code ) 
			return error_code;
    return ERROR_CODE_TRUE;
}

// eof
