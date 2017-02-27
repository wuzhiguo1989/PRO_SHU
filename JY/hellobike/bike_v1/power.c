#include "power.h"
#include "utils.h"
#include "task.h"
#include "beep.h"
#include "key.h"
#include "lm567.h"
 

void power_io_init() {
    // ia
    push_pull(1, 0x04);
    // ib
    push_pull(1, 0x08);

    power_work_disable();

    cpu_wake_up_disable();
}


// alter
void alter_beep_protect()
{
    // CPU WAKE UP
    push_pull(3, 0x40);

    // P32
    pure_input(3, 0x04);
    //push_pull(3, 0x08);

    // P33
    pure_input(3, 0x08);

  //  INT0 = 1;
    // 低电平报警
   // IT0 = 1;   //  设置INT0的中断类型 (1:仅下降沿 0:上升沿和下降沿)
   // EX0 = DISABLE; 

    INT1 = 0;
    // 低电平报警
    IT1 = 0;   //  设置INT0的中断类型 (1:仅下降沿 0:上升沿和下降沿)
    EX1 = ENABLE; 
}



// alter protection
// 中断0
void exint0() interrupt INT0_VECTOR  using 2
{
    // 停止休眠、报警
    speak_alter_pca_init();
    INT_PROC |= ALTER_DUTY;
}

void exint1() interrupt INT1_VECTOR  using 0
{
    // 下降沿中断
    if(!INT1)
    {
        EX1 = DISABLE;
        INT_PROC |= LOCK_DUTY;
    }
    // 上升沿中断 && 不是串口开启
    else
    {
        // 手动关造成的
        if(!lock_fail_from_server)
        {
            EX1 = DISABLE;
            INT_PROC |= LOCK_FAIL_DUTY;
        }
        else{
            EX1 = DISABLE;
            lock_fail_from_server = 0;
            EX1 = ENABLE;
        } 
    } 
}