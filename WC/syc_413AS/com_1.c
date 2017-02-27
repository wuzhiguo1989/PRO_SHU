/*	
*	STC-ISP导入芯片信息至Keil
*	采用STC15W404AS系列
*	假定测试芯片的工作频率为18.432MHz
*/

#include "led.h"
#include "sync.h"
#include "display.h"
#include "shared.h"
#include "e2prom.h"

#define disable_all_interrupt() EA = 0
#define enable_all_interrupt() EA = 1

static void init_system_component();

#define IO_LED_LEFT     0x40	// P3.6
#define IO_LED_RIGHT    0x80	// P3.7
#define IO_P33          0x08    // P3.3
#define IO_INT			0x04	// P3.2



void SendData(u8 dat)
{
    while (!TI);                  
    TI = 0;                        
    SBUF = dat;                    
}


void InitUart()
{
    SCON = 0x5a;                                
    T2L = 0x20;                 
    T2H = 0xfe; 
    AUXR = 0x14;               
    AUXR |= 0x01;
}

void require_delay_init() {
	set_push_pull_output_on3(IO_LED_LEFT);
	set_push_pull_output_on3(IO_LED_RIGHT);
    set_push_pull_output_on3(IO_P33);
	set_input_only_on3(IO_INT);
}

static void
detect_current_state() {
	u8 display_mode = (~P10);
	display_mode = display_mode << 1 | (~P11);
	display_mode = display_mode << 1 | (~P12);
	display_mode = display_mode << 1 | (~P13);
    // display_mode = 0;
	if(display_mode == 0x00)
	{
		display_mode_set = bi_directional_storbing;
		// 从EEPROM取值
		display_mode_logic = EEPROM_read(IAP_ADDRESS);
		if(!display_mode_logic || display_mode_logic >= 13)
			display_mode_logic = 2;
	}
    else if(display_mode == 0x0F)
    {
        display_mode_set = display_mode;
        display_mode_logic = 2;
    }
	else
		display_mode_set = display_mode;
	display_mode_set_changed();  
}

void main()
{
	u8 time_counter_for_cycle = 4;
    disable_all_interrupt();
	//设置输出口、中断
	require_delay_init();
	InitUart();
	init_system_component();
	//初始化端口
	P33 = 0;
	enable_all_interrupt();
	//检测当前状态
	detect_current_state();
    while(1) {
    	// Feed dog
    	reset_watch_dog();
    	// 驱动灯光开关
 //   	toggle_once();
   		// 5ms system tick-tock
    	if (INT_PROC & TICK_DUTY) {
    		cycle_based_adjust(counter_for_cycle);
            INT_PROC &= ~TICK_DUTY;
 //   		continue;
    	}

    	if (INT_PROC & EXINT_PROC) {
            EX0 = 0;
            SendData(time_counter>>8);
 //           SendData(time_counter);
    		if(time_counter <= 2000)
            {
    			if(display_mode_set == combination)
                {
                    -- time_counter_for_cycle;
                }
                // 循环四次跳至下一个功能
                if(!time_counter_for_cycle)
                {
                    display_mode_logic++;
                    time_counter_for_cycle = 4;
                }
                if(display_mode_logic == 13)
                    display_mode_logic = 1;
 //               SendData(display_mode_logic);
                display_mode_set_changed();
            }
    		// if(display_mode_set == bi_directional_storbing && (time_counter >> 8)!= 0x26)
            if(display_mode_set == bi_directional_storbing)
    		{
    			if(time_counter >= 50000 && time_counter < 500000){
    				display_mode_logic ++;
                    if(display_mode_logic == 13)
                        display_mode_logic = 1;
                }
    			if(time_counter >= 500000)
    				display_mode_logic = 2;
                EEPROM_SectorErase(IAP_ADDRESS);
    			EEPROM_write(IAP_ADDRESS, display_mode_logic);
//                SendData(display_mode_logic);
    			display_mode_set_changed();
    		}
    		time_counter = 0;
    		INT_PROC &= ~EXINT_PROC;
            EX0 = 1;
    		continue;
    	}
    }
}

static void
init_system_component() {
	init_sync_timer();
	init_ac_driver();
}

