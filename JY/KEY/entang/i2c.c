#include "i2c.h"
#include "stc15wxxxx.h"
#include "ble.h"

/***********************************************************************************
 * Function Prototypes
 ***********************************************************************************/
void _i2c_start(void);
void _i2c_stop(void);
void _i2c_ack_send(void);
unsigned char _i2c_ack_detect(void);
unsigned char _i2c_write_byte(unsigned char);
unsigned char _i2c_read_byte(void);


void Delay_us()
{
	unsigned char i, j;

    i = 24;
    j = 85;
    do
    {
        while (--j);
    } while (--i);
}


void I2C_DELAY()
{		    
	Delay_us();
}
void I2C_DELAY_LONG()
{	
	Delay_us();
}


void _i2c_start(void)
{
	SDA_HIGH();
	I2C_DELAY();
	I2C_DELAY();
	SCL_HIGH();
	I2C_DELAY();
	I2C_DELAY();
	SDA_LOW();
	I2C_DELAY();
	SCL_LOW();
	I2C_DELAY();
}

void _i2c_stop(void)
{
	SDA_LOW();
	I2C_DELAY();
	SCL_HIGH();
	I2C_DELAY();
	SDA_HIGH();
	I2C_DELAY_LONG();
}

unsigned char _i2c_write_byte(unsigned char dat)
{
	unsigned char i;

	for(i = 0; i< 8; i++)
	{
		if((dat << i) & 0x80)
		{
			SDA_HIGH();
		}
		else
		{
			SDA_LOW();
		}
		I2C_DELAY();
		SCL_HIGH();
		I2C_DELAY();
		SCL_LOW();
		I2C_DELAY();
	}

	if(_i2c_ack_detect()) {
		return ERROR_CODE_FALSE;
	}
	return ERROR_CODE_TRUE;
}



unsigned char _i2c_ack_detect(void)
{
	
	I2C_DELAY();
	SCL_HIGH();
	I2C_DELAY();
	if (SDA)
		return ERROR_CODE_FALSE; 
	I2C_DELAY();
	SCL_LOW();
	return ERROR_CODE_TRUE; // true
}

void _i2c_ack_send(void)
{
	SDA_LOW();
	I2C_DELAY();
	SCL_HIGH();
	I2C_DELAY();
	SCL_LOW();
	I2C_DELAY();
}

void _i2c_nack_send(void)
{
	SDA_HIGH();
	I2C_DELAY();
	SCL_HIGH();
	I2C_DELAY();
	SCL_LOW();
	I2C_DELAY();
}


unsigned char _i2c_read_byte(void)
{
	unsigned char i, dat;
	dat = 0;
	
	for(i = 0; i< 8; i++){
		dat <<= 1;
		I2C_DELAY();
		SCL_HIGH();
		I2C_DELAY();
		if (SDA) 
			dat |= 0x01;
		SCL_LOW();
		I2C_DELAY();
	}
	_i2c_nack_send();
	return dat;
}

unsigned char _i2c_read_byte_last(void)
{
	unsigned char i, dat;
	dat = 0;
	SDA_HIGH();
	for(i = 0; i< 8; i++){
		dat <<= 1;
		I2C_DELAY();
		SCL_HIGH();
		I2C_DELAY();
		if (SDA) 
			dat |= 0x01;
		SCL_LOW();
		I2C_DELAY();
	}
	 _i2c_nack_send();
	return dat;
}

unsigned char _i2c_write(unsigned char device_addr, unsigned char sub_addr, unsigned char *buff, int ByteNo)
{
	unsigned char i;
	_i2c_start();
	I2C_DELAY();
	if(_i2c_write_byte(device_addr)) {
		_i2c_stop();
		return ERROR_CODE_WRITE_ADDR;
	}
	
	if(_i2c_write_byte(sub_addr)) {
		_i2c_stop();
		return ERROR_CODE_WRITE_ADDR;
	}
	
	for(i = 0; i<ByteNo; i++) {
		if(_i2c_write_byte(buff[i])) {
			_i2c_stop();
			return ERROR_CODE_WRITE_DATA;
		}
	}

	I2C_DELAY();
	_i2c_stop();
	I2C_DELAY_LONG();
	return ERROR_CODE_TRUE;
}

unsigned char _i2c_read(unsigned char device_addr, unsigned char sub_addr, unsigned char *buff, int ByteNo)
{
	
	unsigned char i;
	_i2c_start();
	I2C_DELAY();

	if(_i2c_write_byte(device_addr)) {
		_i2c_stop();
		return ERROR_CODE_READ_ADDR;
	}
	
	if(_i2c_write_byte(sub_addr)) {
		_i2c_stop();
		return ERROR_CODE_READ_ADDR;
	}
	//_i2c_stop();
	_i2c_start();
	I2C_DELAY();
	if(_i2c_write_byte(device_addr+1)) {
		_i2c_stop();
		return ERROR_CODE_READ_ADDR;
	}

	for(i = 0; i<(ByteNo-1); i++) 
		buff[i] = _i2c_read_byte();
	buff[ByteNo-1] = _i2c_read_byte_last();
	
	I2C_DELAY();
	I2C_DELAY_LONG();
	_i2c_stop();
	I2C_DELAY_LONG();
	return ERROR_CODE_TRUE;
	
}

/* EOF */

