#include "beep.h"
#include "def.h"
#include "share.h"
#include "intrins.h"

u16 pca_next_target = 0;


//0：MUTE
//1：解锁  600hz
//2：寻车  600hz
//3：关锁  800\500hz(响一声100、100)
//4：报警1 800\500hz
//5：报警2 800\500hz
//6：启动  500hz
//7: 喇叭  600hz
//8: 闪灯  1000hz
u16 pca_next_param[2] = {T4kHz, T1000Hz};



void speak_beep_enable()
{
    if(!MUTE)
        SPEAK_TURN = 0;
    else
        SPEAK_TURN = ~SPEAK_TURN;

}

void init_beep_io()
{
	push_pull(1, 0x01);
    push_pull(1, 0x02);
}


// void detect_buzzer_io()
// {
// 	if (!BUZZER)
//     {
//         mute_beep_enable();
//         speak_mode_set |= 0x0100;
//         speak_mode = buzzer;
        
//     }
//     else
//         speak_mode_set &= ~0x0100;

// }




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
//TODO：解决按键中断speak持续一个周期
void pca_isr() interrupt 7 using 1 
{
    CCF0 = 0; 		// 	清中断标志
    if(!speak_mode_set){
        speak_beep_disable(); 
        mute_beep_disable();
        speak_mode = mute;
        pca_next_target =  pca_next_target + pca_next_param[speak_mode];
        CCAP0L = pca_next_target;
        CCAP0H = pca_next_target >> 8;       
    } else {
        speak_beep_enable();
        pca_next_target =  pca_next_target + pca_next_param[speak_mode];
        CCAP0L = pca_next_target;
        CCAP0H = pca_next_target >> 8;
   }
}
