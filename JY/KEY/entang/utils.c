#include "utils.h"
#include "e2prom.h"


void Store_Password(u8 *data_password, u8 data_from, u8 data_end)
{
	u8 i;
	EEPROM_SectorErase(ADDRESS_FOR_PASSWORD);
	for(i = data_from; i <= data_end; i++)
		EEPROM_write(ADDRESS_FOR_PASSWORD + (i - data_from), data_password[i]);
}

void Read_Password(u8 *send_data, u8 data_from, u8 data_end)
{
	u8 i;
	for(i = data_from; i <= data_end; i++)
		send_data[i] = EEPROM_read(ADDRESS_FOR_PASSWORD + (i - data_from));
}

void Store_Timestamp(u8 *data_timestamp, u8 data_from, u8 data_end)
{
	u8 i;
	EEPROM_SectorErase(ADDRESS_FOR_TIMESTAMP);
	for(i = data_from; i <= data_end; i++)
		EEPROM_write(ADDRESS_FOR_TIMESTAMP + (i - data_from), data_timestamp[i]);
}

u32 Read_Timestamp_High()
{
	u8 i;
	u32 counter = 0;
	for(i = 0; i < 4; i++)
		counter = counter << 8 | EEPROM_read(ADDRESS_FOR_TIMESTAMP + i);
	return counter;
}

u32 Read_Timestamp_Low()
{
	u8 i;
	u32 counter = 0;
	for(i = 4; i < 8; i++)
		counter = counter << 8 | EEPROM_read(ADDRESS_FOR_TIMESTAMP + i);
	return counter;
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