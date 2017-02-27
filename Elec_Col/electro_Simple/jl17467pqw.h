#ifndef __JL17467PQW_H_
#define __JL17467PQW_H_

#include "stc15wxxxx.h"
#include "tm1727.h"
#include "def.h"

#define SPEED_DECADE_ID        		1
#define SPEED_UNIT_ID          		2
#define SPEED_DECIMAL_ID       		3
#define TIME_HOUR_DECADE_ID    		4
#define TIME_HOUR_UNIT_ID      		5
#define TIME_MINUTE_DECADE_ID  		6
#define TIME_MINUTE_UNIT_ID    		7
#define TEMPERATURE_DECADE_ID  		8
#define TEMPERATURE_UNIT_ID    		9
#define MILEAGE_DECADE_ID  			10
#define MILEAGE_UNIT_ID    			11
#define MILEAGE_DECIMAL_ID  		12
#define MILEAGE_TOTAL_THOUSAND_ID   13
#define MILEAGE_TOTAL_HUNDRED_ID  	14
#define MILEAGE_TOTAL_DECADE_ID    	15
#define MILEAGE_TOTAL_UNIT_ID  		16
#define MILEAGE_TOTAL_DECIMAL_ID   	17

#define set(x, line, column)    Tm1727_Local_Set_Bit(line, column, x)

// S1
#define jt17467pqw_set_faults(x)   set(x, 1, 4)
// S2
#define jt17467pqw_set_bluetooth_linked(x)   set(x, 1, 5)
// S3
#define jt17467pqw_set_power_linked(x)	set(x, 1, 6)
// S4
#define jt17467pqw_set_head_lamps(x)   set(x, 1, 7)
// S5
#define jt17467pqw_set_speed_unit_KMph(x)   set(x, 6, 0)
// S6
#define jt17467pqw_set_speed_unit_mps(x)   set(x, 5, 0)
// S7
#define jt17467pqw_set_speed_decimal_point(x)   set(x, 4, 0)
// S8
#define jt17467pqw_set_time_colon(x)  set(x, 7, 0)
// S9
#define jt17467pqw_set_separator(x)   set(x, 1, 0)  
// S10
#define jt17467pqw_set_left_lamps(x)   set(x, 0, 4)
// S16
#define jt17467pqw_set_display_battery(x)   set(x, 0, 7)
// S15
#define jt17467pqw_set_battery_level_1(x)   set(x, 1, 1)
// S14
#define jt17467pqw_set_battery_level_2(x)   set(x, 0, 5)
// S13
#define jt17467pqw_set_battery_level_3(x)   set(x, 1, 2)
// S12
#define jt17467pqw_set_battery_level_4(x)   set(x, 0, 6)
// S11
#define jt17467pqw_set_battery_level_5(x)   set(x, 1, 3)
// S17
#define jt17467pqw_set_display_power(x)   set(x, 19, 7)
// S23  
#define jt17467pqw_set_power_level_1(x)   set(x, 19, 2)
// S22
#define jt17467pqw_set_power_level_2(x)   set(x, 19, 1)
// S21
#define jt17467pqw_set_power_level_3(x)   set(x, 19, 0)
// S20 
#define jt17467pqw_set_power_level_4(x)   set(x, 19, 4)
// S19
#define jt17467pqw_set_power_level_5(x)   set(x, 19, 5)
// S18
#define jt17467pqw_set_power_level_6(x)   set(x, 19, 6)
// S24
#define jt17467pqw_set_temperature_sign(x) set(x, 19, 3)
// S25
#define jt17467pqw_set_temperature_greater_than_hundred(x)  set(x, 18, 4)
// S26
#define jt17467pqw_set_temperature_unit_celsius(x)  set(x, 17, 4)
// S27
#define jt17467pqw_set_temperature_unit_fahrenheit(x)  set(x, 13, 4)
// S28
#define jt17467pqw_set_mileage_decimal_point(x)  set(x, 14, 4)
// S29
#define jt17467pqw_set_milegae_unit_KM(x)  set(x, 15, 4)
// S30
#define jt17467pqw_set_mileage_unit_mile(x)  set(x, 16, 4)
// S31
#define jt17467pqw_set_mileage_total_decimal_point(x)  set(x, 9, 4)
// S32
#define jt17467pqw_set_mileage_total_unit_KM(x)  set(x, 10, 4)
// S33
#define jt17467pqw_set_mileage_total_unit_mile(x)  set(x, 11, 4)
// S34
#define jt17467pqw_set_right_lamps(x)   set(x, 8, 0) 

void jt17467pqw_set_battery(const u8 battery);
// void jt17467pqw_set_power(const u16 power);
void jt17467pqw_set_speed(const u16 speed);
void jt17467pqw_set_time(const time_t time_display);
void jt17467pqw_set_temperature(const u8 temperature);
void jt17467pqw_set_mileage(const u16 mileage);
void jt17467pqw_set_mileage_total(const u32 mileage_total);


#endif