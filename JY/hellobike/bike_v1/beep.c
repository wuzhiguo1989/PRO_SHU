#include "beep.h"
#include "intrins.h"

u16 idata pca_next_target = 0;
static code u16 const pca_next_param[5] = {T100Hz, T4kHz, T800Hz, T600Hz, T600Hz};

u16 idata pca_param = T4kHz;

volatile bit lock_fail_from_server = 0;
volatile speak_t speak_mode = mute;
volatile lock_st status = off;
// volatile sys_st status;

void speak_alter_pca_init() 
{
   pca_param = pca_next_param[alter_beep];
}

void init_beep_io()
{
    // mid
	push_pull(1, 0x01);
    // max
    push_pull(1, 0x02);
    // buzzer
	push_pull(3, 0x80);

    speak_beep_disable();
}



void speak_init_params() {
    CCON = 0;                       //初始化PCA控制寄存器
                                    //PCA定时器停止
                                    //清除CF标志
                                    //清除模块中断标志
    CL = 0;
    CH = 0;
    pca_next_target =  pca_next_target + pca_next_param[0];
    CCAP0L = pca_next_target;
    CCAP0H = pca_next_target >> 8;
    CMOD = 0x00;                    //设置PCA时钟源
                                    //禁止PCA定时器溢出中断
	CCAPM0 = 0x49;                  //PCA模块0为16位定时器模式
    CR = 1;
}


//PCA中断 speak发声
void pca_isr() interrupt 7 using 1 
{
    CCF0 = 0; 		// 	清中断标志
    if(!speak_mode){
        pca_next_target =  pca_next_target + pca_next_param[speak_mode];
        CCAP0L = pca_next_target;
        CCAP0H = pca_next_target >> 8;       
    } else if(speak_mode == alter_beep) {
        speak_beep_enable();
        pca_next_target =  pca_next_target + pca_param;
        CCAP0L = pca_next_target;
        CCAP0H = pca_next_target >> 8;
        pca_param ++;
        if(pca_param >= T2kHz)
            pca_param = T4kHz;
   } else if(speak_mode == trace_beep || speak_mode == open_beep){
        speak_beep_enable();
        pca_next_target =  pca_next_target + pca_next_param[speak_mode];
        CCAP0L = pca_next_target;
        CCAP0H = pca_next_target >> 8;
   }
}
