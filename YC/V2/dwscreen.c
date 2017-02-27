#include "dwscreen.h"
#include "process.h"
#include "task.h"
#include "alert.h"
#include "typealias.h"
#include "e2prom.h"
#include <stdio.h>
#include <string.h>

workflow_t xdata dw_flow_read;
u32 password = 0x00000000;
u32 password_admin_input = 0x00000000;
u32 password_guest_input = 0x00000000;
u8 page = 0xFF;

u8 last_dw_read;


// // 报警记录时间表
// u8 time_record[5][6] = {
// 	{0, 0, 0, 0, 0, 0},
// 	{0, 0, 0, 0, 0, 0},
// 	{0, 0, 0, 0, 0, 0},
// 	{0, 0, 0, 0, 0, 0},
// 	{0, 0, 0, 0, 0, 0}
// };



#define RESP_BUF_SIZE	128
static char xdata cached_response[RESP_BUF_SIZE];
char xdata *dw_cached_response = cached_response;



// 料满延时停机
u8 time_fan_delay_stop = 10;
// 缺料延时启动
u8 time_fan_delay_full = 20;
// 吸料过程料满中断时间
// u8 time_feed_stop = 2;



// 卸料阀延迟关闭时间
u8 time_feed_intercept = 0;
// 吸料阶段反吹时间
u8 time_spray_wash = 5;
// 吸料时间反吹间隔
u8 time_spray_wash_between = 35;
// 停机前阶段反吹阀反吹次数
u8 counter_for_spray_wash = 1;
// 停机前阶段反吹间隔
u8 time_spray_wash_between_before_stop = 35;
// 停机前阶段反吹时间
u8 time_spray_wash_before_stop = 5;

u8 tem_alter = 20;
u8 tem_stop = 100;

// u8 counter_for_feed_lack = 0;
// u8 time_feed_suction = 20;
// u8 time_feed_discharge = 6;
// u8 time_feed_lack = 50;
// u8 counter_for_lack_alter = 1;
// u8 feed_lack_signal = 1;


u8 time_fan_delay_stop_set = 10;
u8 time_fan_delay_full_set = 20;
// u8 time_feed_suction_set = 20;
// u8 time_feed_discharge_set = 6;
u8 time_feed_intercept_set = 0;
u8 time_spray_wash_set = 5;
u8 time_spray_wash_between_set = 35;
u8 counter_for_spray_wash_set = 1;
// u8 time_feed_lack_set = 50;
// u8 counter_for_lack_alter_set = 1;
u8 tem_alter_set = 20;
u8 tem_stop_set = 100;
// u8 time_feed_stop_set = 2;
// u8 feed_lack_signal_set = 1;
u8 time_spray_wash_between_before_stop_set = 35;
u8 time_spray_wash_before_stop_set = 5;

void send_dat_to_screen(u16 addr, u8 dat)
{
	TX_write2buff(COM[COM2_ID], 0x5A);
	TX_write2buff(COM[COM2_ID], 0xA5);
	TX_write2buff(COM[COM2_ID], 0x05);
	TX_write2buff(COM[COM2_ID], 0x82);
	TX_write2buff(COM[COM2_ID], addr >> 8);
	TX_write2buff(COM[COM2_ID], (u8)addr);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], dat);
}

void send_two_value_to_screen(u16 addr, u8 value_1, u8 value_2)
{
	TX_write2buff(COM[COM2_ID], 0x5A);
	TX_write2buff(COM[COM2_ID], 0xA5);
	TX_write2buff(COM[COM2_ID], 0x07);
	TX_write2buff(COM[COM2_ID], 0x82);
	TX_write2buff(COM[COM2_ID], addr >> 8);
	TX_write2buff(COM[COM2_ID], (u8)addr);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], value_1);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], value_2);
}

void send_counter_for_feed_lack_to_dwscreen(u8 dat)
{
	send_dat_to_screen(ADDR_COUNT_FOR_FEED_LACK, dat);
}

void syc_available_siganl(u8 dat)
{
	send_dat_to_screen(ADDR_COUNT_FOR_AVAILABLE, dat);
}

void syc_signal_start_fan(u8 dat) 
{
	send_dat_to_screen(ADDR_START, dat);
}

void syc_signal_beep()
{
	send_dat_to_screen(ADDR_BEEP_CONTROL, 0);
}

void clean_feed_lack_record()
{
	TX_write2buff(COM[COM2_ID], 0x5A);
	TX_write2buff(COM[COM2_ID], 0xA5);
	TX_write2buff(COM[COM2_ID], 0x33);
	TX_write2buff(COM[COM2_ID], 0x82);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], 0x86);
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


void display_all_para(u16 addr)
{
	TX_write2buff(COM[COM2_ID], 0x5A);
	TX_write2buff(COM[COM2_ID], 0xA5);
	TX_write2buff(COM[COM2_ID], 0x17);
	TX_write2buff(COM[COM2_ID], 0x82);
	TX_write2buff(COM[COM2_ID], addr >> 8);
	TX_write2buff(COM[COM2_ID], addr);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], time_spray_wash);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], time_spray_wash_between);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], counter_for_spray_wash);
	// TX_write2buff(COM[COM2_ID], 0x00);
	// TX_write2buff(COM[COM2_ID], time_feed_suction);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], time_feed_intercept);
	// TX_write2buff(COM[COM2_ID], 0x00);
	// TX_write2buff(COM[COM2_ID], time_feed_discharge);
	// TX_write2buff(COM[COM2_ID], 0x00);
	// TX_write2buff(COM[COM2_ID], time_feed_lack);
	// TX_write2buff(COM[COM2_ID], 0x00);
	// TX_write2buff(COM[COM2_ID], counter_for_lack_alter);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], time_spray_wash_before_stop);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], time_spray_wash_between_before_stop);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], time_fan_delay_stop);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], time_fan_delay_full);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], tem_alter);
	TX_write2buff(COM[COM2_ID], 0x00);
	TX_write2buff(COM[COM2_ID], tem_stop);
	// TX_write2buff(COM[COM2_ID], 0x00);
	// TX_write2buff(COM[COM2_ID], time_feed_stop);
	// TX_write2buff(COM[COM2_ID], 0x00);
	// TX_write2buff(COM[COM2_ID], feed_lack_signal);
}

void updata_data_from_dwscreen()
{
	time_fan_delay_stop_set = time_fan_delay_stop;
	time_fan_delay_full_set = time_fan_delay_full;
	// time_feed_suction_set = time_feed_suction;
	// time_feed_discharge_set = time_feed_discharge;
	time_feed_intercept_set = time_feed_intercept;
	time_spray_wash_set = time_spray_wash;
	time_spray_wash_between_set = time_spray_wash_between;
	counter_for_spray_wash_set = counter_for_spray_wash;
	// time_feed_lack_set = time_feed_lack;
	// counter_for_lack_alter_set = counter_for_lack_alter;
	// time_feed_stop_set = time_feed_stop;
	// feed_lack_signal_set = feed_lack_signal;
	time_spray_wash_before_stop_set = time_spray_wash_before_stop;
	time_spray_wash_between_before_stop_set = time_spray_wash_between_before_stop;
	// if(!feed_lack_signal)
 //        clean_feed_lack_counter();
}



bool update_data_should = NO;
void dw_data_reading(u8 *response) {
		time_spray_wash = response[8];
		time_spray_wash_between = response[10];
		counter_for_spray_wash = response[12];
		// time_feed_suction = response[14];
		time_feed_intercept = response[14];
    	time_spray_wash_before_stop = response[16];
    	time_spray_wash_between_before_stop = response[18];
		// counter_for_lack_alter = response[22];
    	time_fan_delay_stop = response[20];
		time_fan_delay_full = response[22];
		tem_alter = response[24];
		tem_stop = response[26];
		// time_feed_stop = response[28];
		// feed_lack_signal = response[34];
		// 实时更新
		tem_alter_set = tem_alter;
		tem_stop_set = tem_stop;

		update_data_should = true;
}


static bool dw_wf_callback(u8 *response, int len) {
//	dw_data_reading(response, len);
	finish_workflow_before_next_flow(DW_READ_WORKFLOW_ID);
	return true;
}

static void dw_wf_start() {
	dw_flow_read.processing = DW_TIMEOUT;
	dw_flow_read.task_id = 0;
	continue_workflow_at_next_round(DW_READ_WORKFLOW_ID);
}

static void dw_wf_clean() {
	last_dw_read = DW_READ_NONE;
}

static void dw_wf_go(u8 task_id) {
		finish_workflow_before_next_flow(DW_READ_WORKFLOW_ID);
}


int dw_cache_response(char xdata *r) {
	u8 len = COM2.RX_Cnt;
	memset(cached_response, 0, RESP_BUF_SIZE);
	if (len > RESP_BUF_SIZE) return 0;
	// copy r -> cached_response until [find "\0" or finish r]
	// CRASH if: sizeof(r) > sizeof(cached_response)
	memcpy(cached_response, r, sizeof(char) * len);
	return len;
}


// void clean_feed_lack_counter()
// {
// 	counter_for_feed_lack = 0;
// 	send_counter_for_feed_lack_to_dwscreen(counter_for_feed_lack);
// 	alert_for_system &= ~ALTER_LACK_FEED;
// 	clean_feed_lack_record();
// }

void system_protect(u8 *r)
{
    u8 year, month, day;
    year = (r[6] >> 4) * 10 + (r[6] & 0x0F);
    month = (r[7] >> 4) * 10 + (r[7] & 0x0F);
    day = (r[8] >> 4) * 10 + (r[8] & 0x0F);
    if(time_year > year){
    	sigal_available = 0;
    } else if(time_year == year){
    	if(time_month > month){
    		sigal_available = 0;
    	} else if(time_month == month){
    		if(time_day > day){
    			sigal_available = 0;
    		} else {
    			sigal_available = 1;
    		}
    	} else {
    		sigal_available = 1;
    	} 
    } else {
    	sigal_available = 1;
    }
   	TX_write2buff(COM[COM1_ID], r[6]);
    TX_write2buff(COM[COM1_ID], r[7]);
    TX_write2buff(COM[COM1_ID], r[8]);
   	TX_write2buff(COM[COM1_ID], time_year);
    TX_write2buff(COM[COM1_ID], time_month);
    TX_write2buff(COM[COM1_ID], time_day);
    TX_write2buff(COM[COM1_ID], sigal_available);
}



void dw_dispatch_response(u8 xdata *r, int len) {

	if(r[3] == 0x83)
	{
		switch(r[5])
		{
			case COMMAND_CANCLE_AND_BACK:
				count_down_cancel(&system_back_home);
				page_init = 0;
				// page_back_home();
				break;
			case COMMAND_PW_ADM_ACK:
				page_password_admin_input_handle();
				reload_system_back_home();
				page_init = 1;
				break;
			case COMMAND_PW_SET_ACK:
				page_password_setting_handle();
				reload_system_back_home();
				page_init = 1;
				break;
			case COMMAND_PW_RESET:
				password_to_init();
				break;
			case COMMAND_PW_MD_ACK:
				modify_password();
				reload_system_back_home();
				page_init = 1;
				break;
			case COMMAND_PA_RESET:
				setting_to_init();
				break;   
			case COMMAND_PA_MD_ACK:
				update_setting();
				break;     
			case COMMAND_BEEP_CONTROL:
			// 解除当次报警  
				if(r[8])
					INT_PROC |= ALTER_BEEP_CLOSE_DUTY;
			// 重新开始报警
				else
				{
					// 料满不需要报警
					if(alert_for_system & 0x1D)
						INT_PROC |= ALTER_BEEP_DUTY;
				}
				page_init = 0;
				break;	 
			case COMMAND_START:
				start_fan_signal = r[8];
				system_start_or_not();
				page_init = 0;
				break;
			case COMMAND_HELP:
				reload_system_back_home();
				page_init = 1;
				break;
			case COMMAND_INQUIRY:
				// page_to_param_inqury();
				reload_system_back_home();
				page_init = 1;
				break; 
			case COMMAND_NEXT_PAGE:
				reload_system_back_home();
				page_init = 1;
				break; 
			case COMMAND_LAST_PAGE:
				reload_system_back_home();
				page_init = 1;
				break;
			case COMMAND_PASSWORD_MANAGEMENT:
				page_password_admin_handle();
				reload_system_back_home();
				page_init = 1;
				break;
			case COMMAND_LOGIN_ON:
				login_on_setting_handle();
				reload_system_back_home();
				page_init = 1;
				break;
			case COMMAND_LOGIN_OUT:
				login_off_current_user();
				page_init = 0;
				break;
			case ADDR_PW_INPUT:
				password = r[7] << 24 | r[8] << 16 | r[9] << 8 | r[10];
				page_init = 1;
				break;
			case ADDR_PW_ADMIN_INPUT:
				password_admin_input = r[7] << 24 | r[8] << 16 | r[9] << 8 | r[10];
				password_guest_input = r[11] << 24 | r[12] << 16 | r[13] << 8 | r[14];
				page_init = 1;
				break;
			case COMMAND_TEST_SPRAY_WASH:
				test_spray_wash_process(r[8]);
				reload_system_back_home();
				page_init = 1;
				break;
			case COMMAND_TEST_SUCTION:
				test_suction_process(r[8]);
				reload_system_back_home();
				page_init = 1;
				break;
			case COMMAND_TEST_INTERCEPT:
				test_intercept_process(r[8]);
				reload_system_back_home();
				page_init = 1;
				break;
			case COMMAND_TEST_BEEP:
				test_beep_process(r[8]);
				reload_system_back_home();
				page_init = 1;
				break;
			case COMMAND_TEST_FAN:
				test_fan_process(r[8]);
				reload_system_back_home();
				page_init = 1;
				break;
			case COMMAND_TEST_REST:
				test_rest_process();
				reload_system_back_home();
				page_init = 1;
				break;
			case  COMMAND_TEST_BY_HAND:
				// page_to_test_hand();
				reload_system_back_home();
				page_init = 1;
				break;
			case  COMMAND_RECORD_INQURY:
				// page_to_record_inqury();
				reload_system_back_home();
				page_init = 1;
				break;
			case INDEX_FOR_ADMIN_SET:
				dw_data_reading(r);
				// 机器没有运行直接更新
				if(!start_fan_signal)
					updata_data_from_dwscreen();
				page_init = 1;
				break;
			case COMMAND_TEST_CANCLE:
				test_rest_process();
				page_init = 0;
				break;
			case COMMAND_REST_STATUS:
			// clean feed_lack history
				// counter_for_feed_lack = 0;
			 // 	send_counter_for_feed_lack_to_dwscreen(counter_for_feed_lack);
			 // 	alert_for_system &= ~ALTER_LACK_FEED;
			 	clean_feed_lack_record();
				break;
			case COMMAND_CANCLE_PW_INPUT:
				password_input_cancle();
				break;
			case COMMAND_CANCLE_PW_MD:
				password_modify_cancle();
				break;
			case COMMAND_CANCLE_PA_INPUT:
				param_set_cancle();
				break;
			case COMMAND_TIME_SET:
				reload_system_back_home();
				page_init = 1;
				break;
			case COMMAND_FEED_LACK_CONTROL:
				// feed_lack_signal = r[8];
				// store_all_para();
				// update_data_should = true;
				break;
			case COMMAND_VALIABLE_CONTROL:
				reload_system_back_home();
				// TX_write2buff(&COM1, r[8]);
				if(!r[8])
				// 取消设置
					reset_available_set();
				else
				// 使用设置
					update_avaiable_deadline();
				break;
			case INDEX_FOR_DEADLINE_SET:
				store_available_set(r);
				break;
			case COMMAND_VALIABLE_ACK:
				// 返回首页、注销登陆
				every_minute_no_identify_call_back();
				page_init = 0;
				break;
			default:
				break;
		}      				
	}
	else if(r[3] == 0x81)
	{
		switch(r[4])
		{
			case REGISTER_TIME:
			{
				if(!record){
					system_protect(r);
					page_back_home();
				}
				else
				{
					u8 i, record_code, mask = 0x01;
					record_code = record;
					for(i = 0; i < 5; i++)
					{
						if(record_code & mask)
						{
						// 报警记录发送
							TX_write2buff(COM[COM2_ID], 0x5A);
							TX_write2buff(COM[COM2_ID], 0xA5);
							TX_write2buff(COM[COM2_ID], 0x09);
							TX_write2buff(COM[COM2_ID], 0x82);
							TX_write2buff(COM[COM2_ID], 0x00);
							TX_write2buff(COM[COM2_ID], 0x80 + i * 6);
							TX_write2buff(COM[COM2_ID], r[6]);
							TX_write2buff(COM[COM2_ID], r[7]);
							TX_write2buff(COM[COM2_ID], r[8]);
							TX_write2buff(COM[COM2_ID], r[10]);
							TX_write2buff(COM[COM2_ID], r[11]);
							TX_write2buff(COM[COM2_ID], r[12]);
						}
						mask <<= 1;
					}

				}
				break;
			}
			case REGISTER_PAGE:
				page = r[7]; 
				page_init = 1;
				break;
			default:
				break;
		}
	}
}

void Delay5ms()		//@24.000MHz
{
	unsigned char i, j;

	i = 117;
	j = 184;
	do
	{
		while (--j);
	} while (--i);
}


void dw_screen_init()
{
	display_all_para(INDEX_FOR_INQUIRY);
	Delay5ms();
	display_all_para(INDEX_FOR_ADMIN_SET);
}


void dw_init() {
	dw_flow_read.uart_id = DW_UART_ID;
	dw_flow_read.workflow_id = DW_READ_WORKFLOW_ID;
	dw_flow_read.task_id = TASK_ID_NONE;
	dw_flow_read.processing = 0;
	dw_flow_read.callback = dw_wf_callback;
	dw_flow_read.start = dw_wf_start;
	dw_flow_read.go = dw_wf_go;
	dw_flow_read.clean = dw_wf_clean;
	dw_flow_read.timeout = NULL;
	last_dw_read = DW_READ_NONE;
	workflow_registration[DW_READ_WORKFLOW_ID] = &dw_flow_read;
}
