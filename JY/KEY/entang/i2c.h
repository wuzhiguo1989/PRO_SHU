#ifndef __I2C_H_
#define __I2C_H_

#include "stc15wxxxx.h"


/***********************************************************************************
 *#defines
 ***********************************************************************************/

#define SCL		P23
#define SDA		P22

#define SCL_LOW()		SCL = 0;
#define SCL_HIGH()		SCL = 1;
#define SDA_LOW()		SDA = 0;
#define SDA_HIGH()		SDA = 1;
 
#define ERROR_CODE_TRUE			0
#define ERROR_CODE_FALSE		1
#define ERROR_CODE_WRITE_ADDR	10
#define ERROR_CODE_WRITE_DATA	20
#define ERROR_CODE_READ_ADDR	30
#define ERROR_CODE_READ_DATA	40
#define ERROR_CODE_START_BIT	50
#define ERROR_CODE_APROCESS		60
#define ERROR_CODE_DENY			70




/***********************************************************************************
 * External Function
 ***********************************************************************************/

extern unsigned char _i2c_write(unsigned char , unsigned char, unsigned char *, int);
extern unsigned char _i2c_read(unsigned char , unsigned char, unsigned char *, int);
  
#endif

/* EOF */

