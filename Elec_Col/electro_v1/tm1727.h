#ifndef __TM1727_H_
#define __TM1727_H_

#include "stc15wxxxx.h"


/********************定义控制端口**********************/

#define SDA P35
#define SCL P34

#define RAM_START_ADDRESS 0xC0

void Tm1727_Local_Set_Bit(const u8 line, const u8 column, const u8 value_bit);
void display_clean();
void display(u8 add, u8 *ram_code);
# endif