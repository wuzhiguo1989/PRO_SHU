# include "jl17467pqw.h"
# include "share.h"
# include "adc.h"

#define FIRST_NOT_DISPLAY  10

static code u8 const digit_code_line_location[17] ={
  // 1, 2, 3, 4, 5, 6, 7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17
     2, 3, 4, 5, 6, 7, 8, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9   // ABCDEFG
};

static code u8 const digit_code_column_location[7][17] = {
  // 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17
    {7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},     // A
    {6, 6, 6, 6, 6, 6, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},     // B
    {5, 5, 5, 5, 5, 5, 5, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},     // C
    {4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},     // D 
    {1, 1, 1, 1, 1, 1, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7},     // E
    {3, 3, 3, 3, 3, 3, 3, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},     // F 
    {2, 2, 2, 2, 2, 2, 2, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6}      // G
};

// GFEDCBA
static code u8 const digit_transfer_code[11] = {
  //   0,    1,    2,    3,    4,    5,    6,    7,    8,    9,  FIRST_NOT_DISPLAY
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x00
};



static void jt17467pqw_set_digit(const u8 digit, const u8 id)
{
	u8 flag;
  u8 mask = 0x01, t_code = digit_transfer_code[digit];
  u8 iterator;
  for (iterator = 0; iterator < 7; ++iterator) {
      flag = t_code & mask;
      set(flag, digit_code_line_location[id-1], digit_code_column_location[iterator][id-1]);
      t_code >>= 1;
    }
}

// 36v
static code u8 const battery_level[4] ={32,  33,  34,  35};
// 48v
// static code u8 const battery_level[4] ={ 43,  45,  47,  49};

static void jt17467pqw_set_battery_level(const u8 level)
{
  if(level == 1)
  {
    jt17467pqw_set_battery_level_1(time_colon_icon);
    jt17467pqw_set_battery_level_2(0);
    jt17467pqw_set_battery_level_3(0);
    jt17467pqw_set_battery_level_4(0);
    jt17467pqw_set_battery_level_5(0);
  }
  else
  {
    jt17467pqw_set_battery_level_1(1);
    jt17467pqw_set_battery_level_2(1);
    jt17467pqw_set_battery_level_3(level < 3 ? 0 : 1);
    jt17467pqw_set_battery_level_4(level < 4 ? 0 : 1);
    jt17467pqw_set_battery_level_5(level < 5 ? 0 : 1);
  }
}

void jt17467pqw_set_battery(const u8 power)
{
  jt17467pqw_set_display_battery(1);
  if(power <= battery_level[0]){
    jt17467pqw_set_battery_level(1);
  } else if(power > battery_level[0] && power <= battery_level[1]){
    jt17467pqw_set_battery_level(2);
  } else if(power > battery_level[1] && power <= battery_level[2]){
    jt17467pqw_set_battery_level(3);
  } else if(power > battery_level[2] && power <= battery_level[3]){
    jt17467pqw_set_battery_level(4);
  } else if(power > battery_level[3]){
    jt17467pqw_set_battery_level(5);
  }
}

static code u16 const power_level[6] ={0x0005,  0x0114,  0x0228,  0x033C,  0x0450, 0x0564};

static void jt17467pqw_set_power_level(const u8 level)
{
    jt17467pqw_set_power_level_1(level < 1 ? 0 : 1);
    jt17467pqw_set_power_level_2(level < 2 ? 0 : 1);
    jt17467pqw_set_power_level_3(level < 3 ? 0 : 1);
    jt17467pqw_set_power_level_4(level < 4 ? 0 : 1);
    jt17467pqw_set_power_level_5(level < 5 ? 0 : 1);
    jt17467pqw_set_power_level_6(level < 6 ? 0 : 1);
}

void jt17467pqw_set_power(const u16 power)
{
  jt17467pqw_set_display_power(1);
  if(power < power_level[0]){
    jt17467pqw_set_power_level(0);
  } else if(power >= power_level[0] && power < power_level[1]){
    jt17467pqw_set_power_level(1);
  } else if(power >= power_level[1] && power < power_level[2]){
    jt17467pqw_set_power_level(2);
  } else if(power >= power_level[2] && power < power_level[3]){
    jt17467pqw_set_power_level(3);
  } else if(power >= power_level[3] && power < power_level[4]){
    jt17467pqw_set_power_level(4);
  } else if(power >= power_level[4] && power < power_level[5]){
    jt17467pqw_set_power_level(5);
  } else if(power >= power_level[5]){
    jt17467pqw_set_power_level(6);
  }
}


// 暂时先考虑KM/h
void jt17467pqw_set_speed(const u16 speed)
{
  jt17467pqw_set_speed_unit_KMph(1);
    //jt17467pqw_set_speed_unit_mps(1);
  if(speed < 100)
    jt17467pqw_set_digit(FIRST_NOT_DISPLAY, SPEED_DECADE_ID);
  else
    jt17467pqw_set_digit(speed/100, SPEED_DECADE_ID);
  jt17467pqw_set_digit((speed%100/10) , SPEED_UNIT_ID);
  jt17467pqw_set_speed_decimal_point(1);
  jt17467pqw_set_digit(speed%10, SPEED_DECIMAL_ID);
}


void jt17467pqw_set_time(const time_t time_display)
{
  jt17467pqw_set_digit(time_display.hour/10, TIME_HOUR_DECADE_ID);
  jt17467pqw_set_digit(time_display.hour%10, TIME_HOUR_UNIT_ID);
  // 闪烁
  jt17467pqw_set_time_colon(time_colon_icon);
  jt17467pqw_set_digit(time_display.minute/10, TIME_MINUTE_DECADE_ID);
  jt17467pqw_set_digit(time_display.minute%10, TIME_MINUTE_UNIT_ID);
}

// 暂时先考虑摄氏度
void jt17467pqw_set_temperature(const u8 temperature)
{
  u8 temperature_value;

  jt17467pqw_set_temperature_unit_celsius(1);
 // jt17467pqw_set_temperature_unit_fahrenheit(1);


  if(temperature < TEMPERATURE_ZERO_VALUE)
  {
    temperature_value = TEMPERATURE_ZERO_VALUE - temperature;
    jt17467pqw_set_temperature_sign(1);

  }
  else
  {
    temperature_value = temperature - TEMPERATURE_ZERO_VALUE;
    jt17467pqw_set_temperature_sign(0);
  }


  if(temperature >= TEMPERATURE_BOIL_VALUE || temperature <= TEMPERATURE_NEGATIVE_BOIL_VALUE)
  {
    jt17467pqw_set_temperature_greater_than_hundred(1);
    temperature_value = temperature_value - 100;
  }
  else
  {
    jt17467pqw_set_temperature_greater_than_hundred(0);
    if(temperature_value < 10)
      jt17467pqw_set_digit(FIRST_NOT_DISPLAY , TEMPERATURE_DECADE_ID);
    else
      jt17467pqw_set_digit(temperature_value/10 , TEMPERATURE_DECADE_ID);
  }

  
  jt17467pqw_set_digit(temperature_value%10 , TEMPERATURE_UNIT_ID);

}



// 暂时先考虑KM
void jt17467pqw_set_mileage(u16 mileage)
{
  jt17467pqw_set_milegae_unit_KM(1);
//jt17467pqw_set_mileage_unit_mile(x);
  if(mileage / 1000)
    mileage = mileage - (mileage / 1000 * 1000);

  if(mileage < 100)
    jt17467pqw_set_digit(FIRST_NOT_DISPLAY, MILEAGE_DECADE_ID);
  else
    jt17467pqw_set_digit(mileage/100, MILEAGE_DECADE_ID);
  jt17467pqw_set_digit(mileage % 100 / 10, MILEAGE_UNIT_ID);
  jt17467pqw_set_mileage_decimal_point(1);
  jt17467pqw_set_digit(mileage % 10, MILEAGE_DECIMAL_ID);
}


// 暂时先考虑KM
void jt17467pqw_set_mileage_total(const u32 mileage_total)
{
  jt17467pqw_set_mileage_total_unit_KM(1);
//  mileage_total += 99000;
//  jt17467pqw_set_mileage_total_unit_mile(x);
  if(mileage_total < 100000)
  {
    if(mileage_total < 10000)
      jt17467pqw_set_digit(FIRST_NOT_DISPLAY, MILEAGE_TOTAL_THOUSAND_ID);
    else
      jt17467pqw_set_digit(mileage_total/10000 , MILEAGE_TOTAL_THOUSAND_ID);
    if(mileage_total < 1000)
      jt17467pqw_set_digit(FIRST_NOT_DISPLAY, MILEAGE_TOTAL_HUNDRED_ID);
    else
      jt17467pqw_set_digit(mileage_total % 10000 / 1000, MILEAGE_TOTAL_HUNDRED_ID);
    if(mileage_total < 100)
      jt17467pqw_set_digit(FIRST_NOT_DISPLAY, MILEAGE_TOTAL_DECADE_ID);
    else
      jt17467pqw_set_digit(mileage_total % 1000 / 100, MILEAGE_TOTAL_DECADE_ID); 
    jt17467pqw_set_digit(mileage_total % 100 / 10 , MILEAGE_TOTAL_UNIT_ID);
    jt17467pqw_set_mileage_total_decimal_point(1);
    jt17467pqw_set_digit(mileage_total % 10 , MILEAGE_TOTAL_DECIMAL_ID);
  }
  else
  {
    jt17467pqw_set_digit(mileage_total/100000 , MILEAGE_TOTAL_THOUSAND_ID);
    jt17467pqw_set_digit(mileage_total % 100000 / 10000, MILEAGE_TOTAL_HUNDRED_ID);
    jt17467pqw_set_digit(mileage_total % 10000 / 1000, MILEAGE_TOTAL_DECADE_ID); 
    jt17467pqw_set_digit(mileage_total % 1000 / 100 , MILEAGE_TOTAL_UNIT_ID);
    jt17467pqw_set_mileage_total_decimal_point(0);
    jt17467pqw_set_digit(mileage_total % 100 / 10 , MILEAGE_TOTAL_DECIMAL_ID);
  }
}
