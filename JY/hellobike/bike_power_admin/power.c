#include "power.h"
#include "utils.h"
#include "task.h"

static code u8 const speed_ref_para[4] = {
    10, 25, 40, 50
};


// time_ref_enable + time_ref_disable = 50us
//   0%      100%    50%      40%     30%
static code u16 const time_ref_enable[5] = {
    0xFFFF, 0xFFFF, 0xFFCE, 0xFFD8, 0xFFE2
};

static code u16 const time_ref_disable[5] = {
    0xFFFF, 0xFFFF, 0xFFCE, 0xFFC4, 0xFFBA
};

// static code u16 const time_ref_enable[5] = {
//     0xFFFF, 0xFFFF, 0xFFCE, 0xFFD8, 0xFD44
// };

// static code u16 const time_ref_disable[5] = {
//     0xFFFF, 0xFFFF, 0xFFCE, 0xFFC4, 0xFED4
// };



// volatile speed_t idata speed = 0;
volatile u8 speed = 0;
u16 int0_counter = 0; 

void power_io_init() {
    // P32
    pure_input(3, 0x04);
    // P33
    pure_input(3, 0x08);
    // P55
    push_pull(5, 0x20);

    INT0 = 1;
    // 低电平报警
    IT0 = 0;   //  设置INT0的中断类型 (1:仅下降沿 0:上升沿和下降沿)
    EX0 = ENABLE; 
}

void driver_timer_enable()
{
    Timer2_Stop();
    T2_Load(time_ref_enable[speed]);
    charge_power_enable();
    Timer2_Run();
}

void driver_timer_disable()
{
    Timer2_Stop();
    T2_Load(time_ref_disable[speed]);
    charge_power_disable();
    Timer2_Run();
}



void freq_cal_driver()
{
    u16 rise_fall_cnt = 0;
    charge_power_disable();
    Timer2_Stop();
    rise_fall_cnt = int0_counter;
    int0_counter = 0;
    // 睡眠
    if(rise_fall_cnt == 0){
        speed = 0;
        INT_PROC |= SLEEP_DUTY;
    // 超低速
    } 
    // not hill_start
    if(!INT1){
        if (rise_fall_cnt > 0 && rise_fall_cnt <= speed_ref_para[0]){
            // speed = extreme_slow;
            speed = 0;
            charge_power_disable();
        } else if (rise_fall_cnt > speed_ref_para[0] && rise_fall_cnt <= speed_ref_para[1]){
            //speed = slow;
            speed = 1;
            // 满格充
            charge_power_enable();
        } else if (rise_fall_cnt > speed_ref_para[1] && rise_fall_cnt <= speed_ref_para[2]){
            //speed = middle;
            speed = 2;
            driver_timer_enable();
        } else if (rise_fall_cnt > speed_ref_para[2] && rise_fall_cnt <= speed_ref_para[3]){
            //speed = fast;
            speed = 3;
            driver_timer_enable();
        } else if (rise_fall_cnt > speed_ref_para[3]){
            //speed = ultrahigh_fast;
            speed = 4;
            driver_timer_enable();
        }
    }

    
}


// 下降沿
// 中断0
void exint0() interrupt INT0_VECTOR  using 1
{
    int0_counter ++;
}
