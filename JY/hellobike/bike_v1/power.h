#ifndef __POWER_H_
#define __POWER_H_

#define IA_POWER P13
#define IB_POWER P12
// K2 中断检测  
#define K2   P32
#define CPU_WAKE_UP P36

#define power_open_work()     IA_POWER = 1; IB_POWER = 0
#define power_lock_work()     IA_POWER = 0; IB_POWER = 1
#define power_work_disable()  IA_POWER = 0; IB_POWER = 0
#define cpu_wake_up_enable()  CPU_WAKE_UP = 1
#define cpu_wake_up_disable()  CPU_WAKE_UP = 0


void power_io_init();
void alter_beep_protect(); 
#endif
