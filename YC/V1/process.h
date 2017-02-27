#ifndef		__PROCESS_H
#define		__PROCESS_H

#include "typealias.h"
#include "uart.h"
#include "workflow.h"


#define  PAGE_HOME1     	  0x01
#define  PAGE_ADMIN_SETTING1  0x05
#define  PAGE_ADMIN_SETTING2  0x07
#define  PAGE_ADMIN_SETTING3  0x09
#define  PAGE_ADMIN_SETTING4  0x16
#define  PAGE_GUEST_SETTING1  0x10
#define  PAGE_GUEST_SETTING2  0x11
#define  PAGE_TIME_SETTING    0x1D
#define  PAGE_PASSWORD_MD     0x12 
#define  PAGE_PASSWORD_LOGIN  0x03
#define  PAGE_PASSWORD_ADMIN  0x0C
#define  PAGE_HELP  		  0x14
#define  PAGE_PARAM_INQURY    0x04
#define  PAGE_TEST_HAND       0x0F
#define  PAGE_RECORD_INQURY   0x0E
#define  PAGE_AVAILABLE_ADMIN 0x1E

#define  COMMAND_INQUIRY   				0x50
#define  COMMAND_PASSWORD_MANAGEMENT	0x51
#define  COMMAND_LOGIN_ON               0x52
#define  COMMAND_HELP                   0x53
#define  COMMAND_NEXT_PAGE              0x54
#define  COMMAND_CANCLE_AND_BACK        0x55
#define  COMMAND_LAST_PAGE              0x56
#define  COMMAND_PW_SET_ACK             0x57
#define  COMMAND_PW_ADM_ACK             0x58
#define  COMMAND_PW_RESET               0x59
#define  COMMAND_PW_MD_ACK              0x5A
#define  COMMAND_PA_RESET               0x5B
#define  COMMAND_PA_MD_ACK              0x5C
#define  COMMAND_LOGIN_OUT              0x5D
#define  COMMAND_TEST_BY_HAND           0x5E
#define  COMMAND_RECORD_INQURY          0x5F
#define  COMMAND_TEST_CANCLE            0x60
#define  COMMAND_REST_STATUS			0x61
#define  COMMAND_CANCLE_PW_INPUT        0x62
#define  COMMAND_CANCLE_PW_MD			0x63
#define  COMMAND_CANCLE_PA_INPUT        0x64
#define  COMMAND_TIME_SET               0x65
#define  COMMAND_VALIABLE_ACK           0x66

#define  COMMAND_BEEP_CONTROL           0x0B
#define  COMMAND_START					0x04
#define  COMMAND_TEST_SPRAY_WASH		0x05
#define  COMMAND_TEST_SUCTION           0x06
#define  COMMAND_TEST_INTERCEPT         0x07
#define  COMMAND_TEST_BEEP              0x08
#define  COMMAND_TEST_FAN               0x09
#define  COMMAND_TEST_REST              0x0A
#define  COMMAND_FEED_LACK_CONTROL		0x4D
#define  COMMAND_VALIABLE_CONTROL       0x4E

#define  RECORD_FEED_FULL             0x0080
#define  RECORD_FEED_LACK             0x0086
#define  RECORD_FAN_OVERLOAD          0x008C
#define  RECORD_TEM_BEEP              0x0092
#define  RECORD_TEM_STOP              0x0098




#define  ADDR_BEEP_CONTROL              0x000B
#define  ADDR_TEM_BEEP                  0x0001
#define  ADDR_LACK_BEEP                 0x0001
#define  ADDR_OVER_BEEP                 0x0001
#define  ADDR_START                     0x0004
#define  ADDR_TEST_SPRAY_WASH           0x0005
#define  ADDR_TEST_SUCTION              0x0006
#define  ADDR_TEST_INTERCEPT            0x0007
#define  ADDR_TEST_BEEP                 0x0008
#define  ADDR_TEST_FAN                  0x0009
#define  ADDR_TEST_REST                 0x000A

#define  ADDR_FEED_LACK_CONTROL         0x004D
#define  ADDR_COUNT_FOR_AVAILABLE       0x004E


#define  ADDR_PW_INPUT					0x0010
#define  ADDR_PW_ADMIN_INPUT            0x0014
#define  ADDR_PW_GUEST_INPUT			0x0018

#define  ADDR_CARTOON                   0x0030

#define INDEX_FOR_INQUIRY               0x0020
#define INDEX_FOR_ADMIN_SET             0x0040
#define INDEX_FOR_GUEST_SET             0x0070
#define INDEX_FOR_DEADLINE_SET          0x00B0

#define REGISTER_TIME  0x20
#define REGISTER_PAGE  0x03

typedef enum feed_mode {
	idle = 0,
	fan_work,
	suction,
	discharge,
	full,
	intercept,
	wait_for_discharge_end
} feed_s;

extern feed_s feed_status;

typedef enum identified_mode {
	identified_none = 0,
	admin,
	guest,
	super
} id_m;

extern id_m identified_status;

// page on home flag
extern bool page_init;


void turn_to_page(u8 page);
void page_back_home();
void page_password_admin_handle();
void page_password_setting_handle();
void password_to_init();
void modify_password();
void setting_to_init();
void update_setting();
void system_start_or_not();
void login_off_current_user();
void page_password_admin_input_handle();
void login_on_setting_handle();

// test
void test_reset_para(u16 addr);
void test_spray_wash_process(u8 value);
void test_suction_process(u8 value);
void test_intercept_process(u8 value);
void test_beep_process(u8 value);
void test_fan_process(u8 value);
void test_rest_process();

// system
void every_minute_no_operate_call_back();
void every_minute_no_identify_call_back();

// cartoon
void system_enter_idle();
void system_enter_fan_work();
void system_enter_sunction();
void system_enter_discharge();
void system_enter_alter_feed_full();
void system_enter_intercept();
void system_enter_spray_wash();

void every_second_update_all_para();


#define page_to_param_inqury() turn_to_page(PAGE_PARAM_INQURY)
#define page_to_test_hand()	turn_to_page(PAGE_TEST_HAND)
#define page_to_record_inqury() turn_to_page(PAGE_RECORD_INQURY)
#define page_to_help() turn_to_page(PAGE_HELP)

void display_init_deadline(u8 year, u8 month, u8 day);
void update_avaiable_deadline();
void password_input_cancle();
void password_modify_cancle();
void param_set_cancle();


#endif