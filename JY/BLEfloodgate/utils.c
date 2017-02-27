#include "utils.h"
#include "e2prom.h"
// 开闸门和开钥匙柜不会同时出现
// 这两个采用相同的地址存储数据
void Store_Counter_for_Open_key_cabinet(u32 counter)
{
	u8 i;
	EEPROM_SectorErase(ADDRESS_FOR_COUNTER_OPEN_FLOOD_DOOR);
	for(i = 0; i < 4; i++)
		EEPROM_write(ADDRESS_FOR_COUNTER_OPEN_FLOOD_DOOR + i, counter >> (8 * (3 - i)));
}

void Store_Counter_for_Open_flood_door(u32 counter)
{
	u8 i;
	EEPROM_SectorErase(ADDRESS_FOR_COUNTER_OPEN_FLOOD_DOOR);
	for(i = 0; i < 4; i++)
		EEPROM_write(ADDRESS_FOR_COUNTER_OPEN_FLOOD_DOOR + i, counter >> (8 * (3 - i)));
}

u32 Read_Counter_for_Open_key_cabinet()
{
	u8 i;
	u32 counter = 0;
	for(i = 0; i < 4; i++)
		counter = counter << 8 | EEPROM_read(ADDRESS_FOR_COUNTER_OPEN_KEY_CLIBE + i);
	return counter;
}

u32 Read_Counter_for_Open_flood_door()
{
	u8 i;
	u32 counter = 0;
	for(i = 0; i < 4; i++)
		counter = counter << 8 | EEPROM_read(ADDRESS_FOR_COUNTER_OPEN_FLOOD_DOOR + i);
	return counter;
}



// 以下三个操作数共用一个扇区
// flag: 1-> counter_for_write_A; 
//       2-> counter_for_change_blename;
void Store_Counter_for_Operate_A_B_and_Blename(u32 counter, u8 flag)
{
	u8 i;
	u32 counter_for_write_A, counter_for_change_blename;
	if(flag == 1){
		counter_for_write_A = counter;
		counter_for_change_blename = Read_Counter_for_change_BLEname();
	}else if(flag == 2){
		counter_for_write_A = Read_Counter_for_Write_A();
		counter_for_change_blename = counter;
	}
	EEPROM_SectorErase(ADDRESS_FOR_COUNTER_WRITE_A); 
	for(i = 0; i < 4; i++)
	{
		EEPROM_write(ADDRESS_FOR_COUNTER_WRITE_A + i, counter_for_write_A >> (8 * (3 - i)));
		EEPROM_write(ADDRESS_FOR_COUNTER_CHANGE_BLE_NAME + i, counter_for_change_blename >> (8 * (3 - i)));
	}
}


void Store_Counter_for_Read_A_and_B(u32 counter)
{
	u8 i;
	EEPROM_SectorErase(ADDRESS_FOR_COUNTER_READ_A_AND_B);
	for(i = 0; i < 4; i++)
		EEPROM_write(ADDRESS_FOR_COUNTER_READ_A_AND_B + i, counter >> (8 * (3 - i)));
}


u32 Read_Counter_for_Read_A_and_B()
{
	u8 i;
	u32 counter = 0;
	for(i = 0; i < 4; i++)
		counter = counter << 8 | EEPROM_read(ADDRESS_FOR_COUNTER_READ_A_AND_B + i);
	return counter;
}

u32 Read_Counter_for_Write_A()
{
	u8 i;
	u32 counter = 0;
	for(i = 0; i < 4; i++)
		counter = counter << 8 | EEPROM_read(ADDRESS_FOR_COUNTER_WRITE_A + i);
	return counter;
}

u32 Read_Counter_for_change_BLEname()
{
	u8 i;
	u32 counter = 0;
	for(i = 0; i < 4; i++)
		counter = counter << 8 | EEPROM_read(ADDRESS_FOR_COUNTER_CHANGE_BLE_NAME + i);
	return counter;

}


// A、B共用一个扇区
// B的值唯一
void Store_A(u8 *data_A, u8 data_from, u8 data_end)
{
	u8 i, data_B[4];
	 for(i = 0; i < 4; i++)
	 	data_B[i] = EEPROM_read(ADDRESS_FOR_B + i);
	EEPROM_SectorErase(ADDRESS_FOR_A);
	for(i = data_from; i <= data_end; i++)
	{
		EEPROM_write(ADDRESS_FOR_A + (i - data_from), data_A[i]);
		EEPROM_write(ADDRESS_FOR_B + (i - data_from), data_B[i - data_from]);
	}
}


void Read_A(u8 *send_data, u8 data_from, u8 data_end)
{
	u8 i;
	for(i = data_from; i <= data_end; i++)
		send_data[i] = EEPROM_read(ADDRESS_FOR_A + (i - data_from));
}

void Read_B(u8 *send_data, u8 data_from, u8 data_end)
{
	u8 i;
	for(i = data_from; i <= data_end; i++)
		send_data[i] = EEPROM_read(ADDRESS_FOR_B + (i - data_from));
}


u8 CRC_Creat(u8 *receive, u8 start, u8 end)
{
	u8 i, j, buffer, store, odd, CRC_creat = 0;
	for(i = start; i <= end; i++)
	{
		odd = 0;
		buffer = receive[i];
		for(j = 0; j < 8; j++)
		{
			store = buffer;
			buffer = buffer >> 1;
			if(store - (buffer << 1))
				odd++;
		}
		if(odd & 0x01)
		{
			CRC_creat = (CRC_creat << 1) + 0;
		}
		else
		{ 
			CRC_creat = (CRC_creat << 1) + 1;
		}
	}
	return  CRC_creat;
}

bool ChecksimpleCRC(u8 *receive, u8 data_from, u8 data_end, u8 CRC_check)
{
	u8 CRC_creat;
	CRC_creat = CRC_Creat(receive, data_from, data_end);
	if(CRC_check == CRC_creat)
		return 1;
	else
		return 0;
}


u16 AND_Creat(u8 *receive, u8 data_from, u8 data_end)
{
	u8 i;
	u16 AND_creat = 0;
	for(i = data_from; i <= data_end; i++)
	{
		AND_creat = AND_creat + receive[i];
	}
	return AND_creat;
}

bool CheckAnd(u8 *receive, u8 data_from, u8 data_end, u8 and_check_1, u8 and_check_2)
{
	u16 and_creat;
	and_creat = AND_Creat(receive, data_from, data_end);
	if(and_check_1 == and_creat >> 8 && and_check_2 == (u8) and_creat)
		return 1;
	else
		return 0;	
}

void Complement_Creat(u8 *receive, u8 data_from, u8 data_end, u8 *complemet_creat, u8 data_start, u8 data_over)
{
	u8 i;
	for(i = data_from; i <= data_end; i++)
		complemet_creat[data_start + i - data_from] = ~ receive[i];
}

bool CheckComplement(u8 *receive, u8 data_from, u8 data_end, u8 *complement_check, u8 data_start, u8 data_over)
{
	u8 i, j = 0;
	for(i = data_from; i <= data_end; i++)
	{
		if(receive[i] == ~ complement_check[data_start + i - data_from])
			j++;
	}
	if(j == data_end - data_from + 1)
		return 1;
	else 
		return 0;

}

void Order_Over_Creat(u8 *receive, u8 data_from, u8 data_end, u8 *order_over_creat, u8 data_start, u8 data_over)
{
	u8 i;
	for(i = data_from; i <= data_end; i++)
		order_over_creat[data_over - i - data_from] = receive[i];
}

bool CheckOrder_Over(u8 *receive, u8 data_from, u8 data_end, u8 *order_over_check, u8 data_start, u8 data_over)
{
	u8 i, j = 0;
	for(i = data_from; i <= data_end; i++)
	{
		if(receive[i] == order_over_check[data_over - i - data_from])
			j++;
	}
	if(j == data_end - data_from + 1)
		return 1;
	else 
		return 0;
} 

void Exclusive_Or_Creat(u8 *receive_1, u8 data_from, u8 data_end, u8 *receive_2, u8 data_start, u8 data_over, u8 *exclusive_or_creat, u8 from, u8 end)
{
	u8 i;
	for(i = data_from; i <= data_end; i++)
		exclusive_or_creat[from + i - data_from] = receive_1[i] ^ receive_2[data_start + i - data_from];
	 
}


bool CheckExclusive_Or(u8 *receive_1, u8 data_from, u8 data_end, u8 *receive_2, u8 data_start, u8 data_over, u8 *exclusive_or_check, u8 from, u8 end)
{
	u8 i, j = 0;
	for(i = data_from; i <= data_end; i++)
	{
		if(exclusive_or_check[from + i - data_from] == receive_1[i] ^ receive_2[data_start + i - data_from])
			j++;
	}
	if(j == data_end - data_from + 1)
		return 1;
	else 
		return 0;
} 