#ifndef		__DWSCREEN_H
#define		__DWSCREEN_H

#include "typealias.h"
#include "uart.h"
#include "workflow.h"

#define DW_UART_ID				COM2_ID

#define DW_TIMEOUT				2
#define DW_TASK_INTERVAL		10


// 喷洗时间(0.0~9.9)
#define ADDR_SPRAY_WASH		 	  0x0000
// 喷洗时间间隔(0.0~9.9)
#define ADDR_SPRAY_WASH_BETWEEN	  0x0001
// 喷洗次数(0~99)
#define ADDR_COUNT_FOR_SPRAY_WASH 0x0002
// 吸料时间(0~99)
#define ADDR_FEED_SUCTION         0x0003
// 截料时间(0~99)
#define ADDR_FEED_INTERCEPT       0x0004
// 放料时间(0~99)
#define ADDR_FEED_DISCHARGE       0x0005
// 缺料检测时间(0.0~9.9)
#define ADDR_FEED_LACK		      0x0006
// 缺料警告次数(0~99)
#define ADDR_COUNT_FOR_LACK_ALERT 0x0007
// 风机关机延迟时间(0~99)
#define ADDR_FAN_DELAY_STOP       0x0008
// 风机料满延迟启动时间(0~99)
#define ADDR_FAN_DELAY_FULL       0x0009
// 报警温度(-30.0~100.0)
#define ADDR_TEMPERATURE_ALERT    0x000A
// 停机温度(-30.0~100.0)
#define ADDR_TEMPERATURE_STOP     0x000B
// 吸料停机
#define ADDR_FEED_STOP            0x000C

// 环境温度(-30.0~100.0)
#define ADDR_TEMPERATURE_AROUND   0x002E
// 电机温度(-30.0~100.0)
#define ADDR_TEMPERATURE_FAN      0x002F
// 缺料次数(0~99)
#define ADDR_COUNT_FOR_FEED_LACK  0x0032

extern u8 counter_for_feed_lack;

extern bool update_data_should;
extern u8 time_fan_delay_stop_set;
extern u8 time_fan_delay_full_set;
extern u8 time_feed_suction_set;
extern u8 time_feed_discharge_set;
extern u8 time_feed_intercept_set;
extern u8 time_spray_wash_set;
extern u8 time_spray_wash_between_set;
extern u8 counter_for_spray_wash_set;
extern u8 time_feed_lack_set;
extern u8 counter_for_lack_alter_set;
extern u8 tem_alter_set;
extern u8 tem_stop_set;
extern u8 time_feed_stop_set;
extern u8 feed_lack_signal_set;

extern u8 time_fan_delay_stop;
extern u8 time_fan_delay_full;
extern u8 time_feed_suction;
extern u8 time_feed_discharge;
extern u8 time_feed_intercept;
extern u8 time_spray_wash;
extern u8 time_spray_wash_between;
extern u8 counter_for_spray_wash;
extern u8 time_feed_lack;
extern u8 counter_for_lack_alter;
extern u8 tem_alter;
extern u8 tem_stop;
extern u8 time_feed_stop;
extern u8 feed_lack_signal;

extern u32 password;
extern u32 password_admin_input;
extern u32 password_guest_input;
extern u8 page;

extern bool update_data_should;

// #define MODBUS_SECTION_READ_S1		106		// 1: section read ; 06: index from 0 to 6 
// #define MODBUS_SECTION_READ_S2  	179		// 2: section read ; 79: index from 7 to 9
#define DW_READ_NONE	255

extern workflow_t xdata dw_flow_read;


void Delay5ms();
void display_all_para(u16 addr);
void dw_gen_task();
void dw_init();
void dw_screen_init();

void updata_data_from_dwscreen();
void dw_read_data_from_screen();


// void em08_modbus_rtu_read(u8 section_or_single_tag);
// float em08_numeric_reading_get(int idx, bool comsume);
// int em08_modbus_read_response_len(int quantity);
// bool em08_numeric_reading_updated(int idx);

// MARK: UART

// extern char xdata cached_response[RESP_BUF_SIZE];
extern char xdata *dw_cached_response;


void send_two_value_to_screen(u16 addr, u8 value_1, u8 value_2);
void send_counter_for_feed_lack_to_dwscreen(u8 dat);
void syc_signal_start_fan(u8 dat); 
void syc_signal_beep();
int dw_cache_response(char xdata *);
void dw_dispatch_response(char xdata *, int len);
void clean_feed_lack_counter();



#endif