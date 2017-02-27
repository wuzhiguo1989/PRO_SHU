#include "process.h"
#include "uart.h"
#include "dwscreen.h"
#include "task.h"
#include "alert.h"
#include "e2prom.h"


#define STATUS_IDLE       0x01
#define STATUS_FAN_WORK   0x02
#define STATUS_SUCTION    0x04
#define STATUS_INTERCEPT  0x08
#define STATUS_DISCHARGE  0x10
#define STATUS_FEED_FULL  0x20
#define STATUS_SPRAY_WASH 0x40

u8 feed_status;
id_m identified_status = identified_none;
bool page_init = 1;


void turn_to_page(u8 page)
{
	TX_write2buff(COM[COM2_ID], 0x5A);
	TX_write2buff(COM[COM2_ID], 0xA5);
	TX_write2buff(COM[COM2_ID], 0x04);
	TX_write2buff(COM[COM2_ID], 0x80);
	TX_write2buff(COM[COM2_ID], 0x03);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], page);
}



void get_password_from_screen()
{
	TX_write2buff(COM[COM2_ID], 0x5A);
	TX_write2buff(COM[COM2_ID], 0xA5);
	TX_write2buff(COM[COM2_ID], 0x04);
	TX_write2buff(COM[COM2_ID], 0x83);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x10);
	TX_write2buff(COM[COM2_ID], 0x02);
}

void reset_password_input()
{
	TX_write2buff(COM[COM2_ID], 0x5A);
	TX_write2buff(COM[COM2_ID], 0xA5);
	TX_write2buff(COM[COM2_ID], 0x07);
	TX_write2buff(COM[COM2_ID], 0x82);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x10);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x00);	
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x00);
}

#define page_to_admin_setting_first()   turn_to_page(PAGE_ADMIN_SETTING1)
#define page_to_guest_setting_first()   turn_to_page(PAGE_GUEST_SETTING1)
#define page_to_login_on_identify()		turn_to_page(PAGE_PASSWORD_LOGIN)

void page_password_setting_handle_callback()
{
	// reset password screen
	reset_password_input();
	// pass
	if(password == password_adminstrator)
	{
		identified_status = admin;
		page_to_admin_setting_first();
		reload_system_no_identify();
	}
	else if(password == password_guest)
	{
		identified_status = guest;
		page_to_guest_setting_first();
		reload_system_no_identify();
	}
	else 
		identified_status = identified_none;
	password = 0;
}

// password setting
void page_password_setting_handle()
{
	start_simple_workflow(DW_UART_ID, get_password_from_screen, page_password_setting_handle_callback);
}

void login_on_setting_handle()
{
	if(identified_status == admin)
	{
		page_to_admin_setting_first();
		reload_system_no_identify();
	}
	else if(identified_status == guest)
	{
		page_to_guest_setting_first();
		reload_system_no_identify();
	}
	else
		page_to_login_on_identify();
}

// setting init
void setting_to_init()
{
	// read from e2prom
	reset_all_para();

	// init
	update_data_should = true;
	//
	display_all_para(INDEX_FOR_ADMIN_SET);
	display_all_para(INDEX_FOR_INQUIRY);

	page_back_home();
}

void get_param_set_from_screen()
{
	TX_write2buff(COM[COM2_ID], 0x5A);
	TX_write2buff(COM[COM2_ID], 0xA5);
	TX_write2buff(COM[COM2_ID], 0x04);
	TX_write2buff(COM[COM2_ID], 0x83);
	TX_write2buff(COM[COM2_ID], INDEX_FOR_ADMIN_SET >> 8);
	TX_write2buff(COM[COM2_ID], INDEX_FOR_ADMIN_SET);
	TX_write2buff(COM[COM2_ID], 0x0E);
}




void update_setting_callback()
{
	display_all_para(INDEX_FOR_INQUIRY);
    // store the value
    store_all_para();
	update_data_should = true;
	page_back_home();
}


void update_setting()
{
	start_simple_workflow(DW_UART_ID, get_param_set_from_screen, update_setting_callback);
}


void get_deadline_set_from_screen(){
	TX_write2buff(COM[COM2_ID], 0x5A);
	TX_write2buff(COM[COM2_ID], 0xA5);
	TX_write2buff(COM[COM2_ID], 0x04);
	TX_write2buff(COM[COM2_ID], 0x83);
	TX_write2buff(COM[COM2_ID], INDEX_FOR_DEADLINE_SET >> 8);
	TX_write2buff(COM[COM2_ID], INDEX_FOR_DEADLINE_SET);
	TX_write2buff(COM[COM2_ID], 0x03);
}

void display_init_deadline(u8 year, u8 month, u8 day){
	TX_write2buff(COM[COM2_ID], 0x5A);
	TX_write2buff(COM[COM2_ID], 0xA5);
	TX_write2buff(COM[COM2_ID], 0x09);
	TX_write2buff(COM[COM2_ID], 0x82);
	TX_write2buff(COM[COM2_ID], INDEX_FOR_DEADLINE_SET >> 8);
	TX_write2buff(COM[COM2_ID], (u8)INDEX_FOR_DEADLINE_SET);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], year);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], month);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], day);
}

void update_avaiable_deadline(){
    get_deadline_set_from_screen();
}

void update_feed_lack_signal()
{
	update_data_should = true;
}

#define page_to_password_admin() turn_to_page(PAGE_PASSWORD_MD)
#define page_to_password_admin_login_on()  turn_to_page(PAGE_PASSWORD_ADMIN)
#define page_to_available_admin() turn_to_page(PAGE_AVAILABLE_ADMIN)

void page_password_admin_handle_callback()
{
	// reset password screen
	reset_password_input();
	// pass
	if(password == password_adminstrator)
	{
		identified_status = admin;
		page_to_password_admin();
		reload_system_no_identify();
	} else if(password == password_super){
	    identified_status = super;
		page_to_available_admin();
		reload_system_no_identify();
	}
	password = 0;
}

void page_password_admin_input_handle()
{
	start_simple_workflow(DW_UART_ID, get_password_from_screen, page_password_admin_handle_callback);
}

// password admin
void page_password_admin_handle()
{
	if(identified_status == admin)
	{
		page_to_password_admin();
		reload_system_no_identify();
	}
	else
		page_to_password_admin_login_on();
}

void reset_password_modify_input()
{
	TX_write2buff(COM[COM2_ID], 0x5A);
	TX_write2buff(COM[COM2_ID], 0xA5);
	TX_write2buff(COM[COM2_ID], 0x0B);
	TX_write2buff(COM[COM2_ID], 0x82);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x14);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x00);	
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x00);	
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x00);
}

void password_to_init()
{
	reset_password_modify_input();
	// read from e2prom
	reset_all_password();
	// init
	identified_status = identified_none;
	count_down_cancel(&password_no_identified);
	// page_back_home();
}



void page_password_modify_handle_callback()
{
	// reset password screen
	reset_password_modify_input();
	if(!password_admin_input && password_guest_input)
	{
		password_guest = password_guest_input;
		store_all_password();

	} else if(password_admin_input && !password_guest_input)
	{
		password_adminstrator =  password_admin_input;
		store_all_password();

	} else if(password_admin_input && password_guest_input)
	{
		password_guest = password_guest_input;
		password_adminstrator =  password_admin_input;
		store_all_password();
	}

	password_admin_input = 0;
	password_guest_input = 0;
	// 恢复0
	identified_status = identified_none;
	// todo 
	// cancle count down
	count_down_cancel(&password_no_identified); 
	page_back_home();
}


void get_password_modify_from_screen()
{
	TX_write2buff(COM[COM2_ID], 0x5A);
	TX_write2buff(COM[COM2_ID], 0xA5);
	TX_write2buff(COM[COM2_ID], 0x04);
	TX_write2buff(COM[COM2_ID], 0x83);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x14);
	TX_write2buff(COM[COM2_ID], 0x04);
}

void modify_password()
{
	start_simple_workflow(DW_UART_ID, get_password_modify_from_screen, page_password_modify_handle_callback);
}


void get_current_page()
{
	TX_write2buff(COM[COM2_ID], 0x5A);
	TX_write2buff(COM[COM2_ID], 0xA5);
	TX_write2buff(COM[COM2_ID], 0x03);
	TX_write2buff(COM[COM2_ID], 0x81);
	TX_write2buff(COM[COM2_ID], 0x03);
	TX_write2buff(COM[COM2_ID], 0x02);
}


void no_oprate_callback()
{
	// if page in setting, param value should be replaced with param used now
	// solve the problem with display
	if(page == PAGE_ADMIN_SETTING1 || page == PAGE_ADMIN_SETTING2 || page == PAGE_ADMIN_SETTING3 || page == PAGE_ADMIN_SETTING4 || page == PAGE_GUEST_SETTING1 || page == PAGE_GUEST_SETTING2 || page == PAGE_TIME_SETTING)
		display_all_para(INDEX_FOR_ADMIN_SET);
	// if page in password_input
	else if(page == PAGE_PASSWORD_MD)
	{
		password_admin_input = 0;
		password_guest_input = 0;
		reset_password_modify_input();		
	}
	else if(page == PAGE_PASSWORD_LOGIN || page == PAGE_PASSWORD_ADMIN)
	{
		password = 0;
		reset_password_input();
	}
	page_back_home();
	page = 0xFF;
}

// back every minute no operate
void every_minute_no_operate_call_back()
{
	// jude what page is displaying
	start_simple_workflow(DW_UART_ID, get_current_page, no_oprate_callback);
}




void login_off_current_user()
{
	identified_status = identified_none;
	// cancle count down
	count_down_cancel(&password_no_identified);  
}


void every_minute_no_identify_call_back()
{
	login_off_current_user();
}




// test
void test_reset_para(u16 addr)
{
	TX_write2buff(COM[COM2_ID], 0x5A);
	TX_write2buff(COM[COM2_ID], 0xA5);
	TX_write2buff(COM[COM2_ID], 0x05);
	TX_write2buff(COM[COM2_ID], 0x82);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], (u8)addr);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x00);
}



void test_spray_wash_process(u8 value)
{
	if(!start_fan_signal)
	{
		if(!value)
		{
			count_down_cancel(&test_spray_wash);
			spray_wash_disable();
		}
		else
		{
			spray_wash_enable();
			reload_test_spray_wash();
		}
	}
	else
		test_reset_para(ADDR_TEST_SPRAY_WASH);
}

void test_suction_process(u8 value)
{
	if(!start_fan_signal)
	{
		if(!value)
		{
			count_down_cancel(&test_suction);
			discharge_enable();
		}
		else
		{
			suction_enable();
			reload_test_suction();
		}
	}
	else
		test_reset_para(ADDR_TEST_SUCTION);

}


void test_intercept_process(u8 value)
{
	if(!start_fan_signal)
	{
		if(!value)
		{
			count_down_cancel(&test_intercept);
			intercept_disable();
		}
		else
		{
			intercept_enable();
			reload_test_intercept();
		}
	}
	else
		test_reset_para(ADDR_TEST_INTERCEPT);
		
}

void test_beep_process(u8 value)
{
	if(!start_fan_signal)
	{
    	if(!value)
    	{
    		count_down_cancel(&test_beep);
    		INT_PROC |= ALTER_BEEP_CLOSE_DUTY;
    	}
    	else
    	{
    		INT_PROC |= ALTER_BEEP_DUTY;
			reload_test_beep();
		}
	}
	else
		test_reset_para(ADDR_TEST_BEEP);
		
}

void test_fan_process(u8 value)
{
	if(!start_fan_signal)
	{
		if(!value)
    	{
    		count_down_cancel(&test_fan);
    		fan_work_disable();
    	}
    	else
    	{
			fan_work_enable();
			reload_test_fan();
		}
	}
	else
		test_reset_para(ADDR_TEST_FAN);
		
}

void test_reset_all_para()
{
	TX_write2buff(COM[COM2_ID], 0x5A);
	TX_write2buff(COM[COM2_ID], 0xA5);
	TX_write2buff(COM[COM2_ID], 0x0F);
	TX_write2buff(COM[COM2_ID], 0x82);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x05);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x00);
}

void test_rest_process()
{
	valve_disable();
	alert_beep_disable();
	count_down_cancel_test();
	test_reset_all_para();
}

void cartoon_display_change(u8 value)
{
	TX_write2buff(COM[COM2_ID], 0x5A);
	TX_write2buff(COM[COM2_ID], 0xA5);
	TX_write2buff(COM[COM2_ID], 0x05);
	TX_write2buff(COM[COM2_ID], 0x82);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x30);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], value);

}


void system_enter_idle()
{
	feed_status = idle;
	cartoon_display_change(STATUS_IDLE);
}


void system_enter_fan_work()
{
	feed_status = fan_work;
	cartoon_display_change(STATUS_FAN_WORK);
}

void system_enter_sunction()
{
	feed_status = suction;
	cartoon_display_change(STATUS_SUCTION);
}

void system_enter_discharge()
{
	feed_status = discharge;
	cartoon_display_change(STATUS_DISCHARGE);
}

void system_enter_alter_feed_full()
{
	feed_status = full;
	cartoon_display_change(STATUS_FEED_FULL);
}

void system_enter_intercept()
{
	feed_status = intercept;
	cartoon_display_change(STATUS_INTERCEPT);
}

void system_enter_spray_wash()
{
	cartoon_display_change(STATUS_SPRAY_WASH);
}


// go back home
void page_back_home()
{
	// first page
	turn_to_page(PAGE_HOME1);
	count_down_cancel(&system_back_home);
	page_init = 0;
}

void password_input_cancle()
{
	count_down_cancel(&system_back_home);
	reset_password_input();
	login_off_current_user();
	page_init = 0;
}

void password_modify_cancle()
{
	count_down_cancel(&system_back_home);
	reset_password_modify_input();
	reload_system_no_identify();
	page_init = 0;
}

void param_set_cancle()
{
	display_all_para(INDEX_FOR_ADMIN_SET);
	count_down_cancel(&system_back_home);
	reload_system_no_identify();
	page_init = 0;
}