/*  
*   STC-ISP导入芯片信息至Keil
*   采用STC15W4K32S4系列
*   假定测试芯片的工作频率为24.000MHz
*/

#include "utils.h"
#include "sync.h"
#include "task.h"
#include "string.h"
#include "power.h"
#include "utils.h"

static void init_system_component();
static void init_logic_params();



void init_system_component();

void main()
{  
    // stop catching interrupt
    EA = 0;
    init_system_component();
    // begin catching interrupt
    EA = 1;

    INT_PROC = SLEEP_DUTY;  

    while(1) {
        // Feed dog
        reset_watch_dog();
        if(INT_PROC & FRE_CAL_DUTY)
        {
            freq_cal_driver();
            INT_PROC &= ~FRE_CAL_DUTY;
            continue;
        }

        if(INT_PROC & SLEEP_DUTY)
        {
            charge_power_disable();
            INT_PROC &= ~SLEEP_DUTY;
            PCON = 0x02;
            continue;
        }
    }
}


static void init_system_component() {  
    // timer
    init_system_tick();
    init_driver_timer();
	// power
	power_io_init();
    charge_power_disable();
	
}



