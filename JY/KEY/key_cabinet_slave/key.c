#include "key.h"
#include "led.h"
#include "utils.h"
#include "share.h"
#include "ble.h"
#include "string.h"


#define LOW     0
#define HIGH    1

static volatile u32 g_PreKey;   //前次按键值 
static volatile u32 g_NowKey;   //当前按键值 
static volatile u32 g_ShortKeyCode = 0;    //短按键,单次触发 
static volatile u32 g_LongKeyCode = 0;     //长按键,持续触发 

#define config_A_output() \
    push_pull(KA1_Mx, KA1_BIT); \
    push_pull(KA2_Mx, KA2_BIT); \
    push_pull(KA3_Mx, KA3_BIT); \
    push_pull(KA4_Mx, KA4_BIT); \
    push_pull(KA5_Mx, KA5_BIT); \

#define config_B_output() \
    push_pull(KB1_Mx, KB1_BIT); \
    push_pull(KB2_Mx, KB2_BIT); \
    push_pull(KB3_Mx, KB3_BIT); \
    push_pull(KB4_Mx, KB4_BIT); \
    push_pull(KB5_Mx, KB5_BIT); \
    push_pull(KB6_Mx, KB6_BIT);


#define config_A_input() \
    pure_input(KA1_Mx, KA1_BIT); \
    pure_input(KA2_Mx, KA2_BIT); \
    pure_input(KA3_Mx, KA3_BIT); \
    pure_input(KA4_Mx, KA4_BIT); \
    pure_input(KA5_Mx, KA5_BIT); \

#define config_B_input() \
    pure_input(KB1_Mx, KB1_BIT); \
    pure_input(KB2_Mx, KB2_BIT); \
    pure_input(KB3_Mx, KB3_BIT); \
    pure_input(KB4_Mx, KB4_BIT); \
    pure_input(KB5_Mx, KB5_BIT); \
    pure_input(KB6_Mx, KB6_BIT);



// KeyScan for common use without any speedup
// For each pin is separate
// static u16 
// IO_KeyScan_G() {
//     u32 key = 0;
//     // Read Vertical will set KBx = 0
//     config_A_input();
//     config_B_output();
//     KB1 = KB2 = KB3 = KB4 = KB5 = KB6 = LOW;
//     key = KA5;
//     key <<= 1;
//     key |= KA4;
//     key <<= 1;
//     key |= KA3;
//     key <<= 1;
//     key |= KA2;
//     key <<= 1;
//     key |= KA1;
//     key <<= 1;
//     // Read Horizontal will set KAx = 0
//     config_B_input();
//     config_A_output();
//     KA1 = KA2 = KA3 = KA4 = KA5 = LOW;
//     key |= KB6;
//     key <<= 1;
//     key |= KB5;
//     key <<= 1;    
//     key |= KB4;
//     key <<= 1;
//     key |= KB3;
//     key <<= 1;
//     key |= KB2;
//     key <<= 1;
//     key |= KB1;
//     return key;
// }

// 行列扫描的缺陷：当有多个键同时按下时，可能无法检测具体位置
// 不使用行列扫描，单独置行， 求列， 能否求出每个按键的位置？
u32 
IO_KeyScan_G() {
    u32 key = 0x00000003;
    config_B_input();
    config_A_output();

    KA5 = LOW;
    KA1 = KA2 = KA3 = KA4 = HIGH;
    key <<= 1;
    key |= KB6;
    key <<= 1;
    key |= KB5;
    key <<= 1;
    key |= KB4;
    key <<= 1;
    key |= KB3;
    key <<= 1;
    key |= KB2;
    key <<= 1;
    key |= KB1;

    KA4 = LOW;
    KA1 = KA2 = KA3 = KA5 = HIGH;
    key <<= 1;
    key |= KB6;
    key <<= 1;
    key |= KB5;
    key <<= 1;
    key |= KB4;
    key <<= 1;
    key |= KB3;
    key <<= 1;
    key |= KB2;
    key <<= 1;
    key |= KB1;

    KA3 = LOW;
    KA1 = KA2 = KA4 = KA5 = HIGH;
    key <<= 1;
    key |= KB6;
    key <<= 1;
    key |= KB5;
    key <<= 1;
    key |= KB4;
    key <<= 1;
    key |= KB3;
    key <<= 1;
    key |= KB2;
    key <<= 1;
    key |= KB1;

    KA2 = LOW;
    KA1 = KA3 = KA4 = KA5 = HIGH;
    key <<= 1;
    key |= KB6;
    key <<= 1;
    key |= KB5;
    key <<= 1;
    key |= KB4;
    key <<= 1;
    key |= KB3;
    key <<= 1;
    key |= KB2;
    key <<= 1;
    key |= KB1;

    KA1 = LOW;
    KA2 = KA3 = KA4 = KA5 = HIGH;
    key <<= 1;
    key |= KB6;
    key <<= 1;
    key |= KB5;
    key <<= 1;
    key |= KB4;
    key <<= 1;
    key |= KB3;
    key <<= 1;
    key |= KB2;
    key <<= 1;
    key |= KB1;

    return key;
}





//g_NowKey ^ g_PreKey   :边缘触发 
//g_NowKey & (g_NowKey ^ g_PreKey):上升沿触发 
//g_PreKey & (g_NowKey ^ g_PreKey):下降沿触发 
//g_NowKey ^ (g_NowKey ^ g_PreKey):高电平触发(带消抖) 
//触发方式 ^ g_KeyCode :仅更新变化部分按键值 

//1.支持长短按键同时并行触发
//2.将短按键触发判据改为下降沿触发(短按键释放)

void key_scan_task() {
    u32 key = IO_KeyScan_G();
    static u16 CntPlus = 0;
    // key ^= 0xffff;
    // key_state_trigger = key & ( key ^ key_state_hold);
    // key_state_hold = key;
    
    g_PreKey = g_NowKey; 
    g_NowKey = ~key; 
    g_ShortKeyCode = (g_PreKey & (g_NowKey ^ g_PreKey)) ^ g_ShortKeyCode;  // 下降沿触发 

    if (g_NowKey ^ g_PreKey ^ g_NowKey ^ g_LongKeyCode)         // 高电平触发(带消抖) 
    { 
        if(CntPlus++ > LONG_KEY_TIME)
        {                           
            g_LongKeyCode |= g_NowKey;                          //添加长按键值
            if(CntPlus >= 60000)
                  CntPlus = 2000;
        }    
        if(CntPlus > SHORT_KEY_TIME) 
            g_ShortKeyCode &= g_NowKey;

    } 
    else 
    {       
        CntPlus = 0;                                        //无新增按键出现,计数归零 
        g_LongKeyCode &= g_NowKey;                          //剔除已释放的按键
        g_ShortKeyCode &= g_NowKey;
    }
}

// static code u8  const keyboard_map[5][6] = {
//     {1, 2, 3, 4, 5, 6},
//     {7, 8, 9, 10, 11, 12},
//     {13, 14, 15, 16, 17, 18},
//     {19, 20, 21, 22, 23, 24},
//     {25, 26, 27, 28, 29, 30}
// };

// void check_key_location() {
//     u8 vertical, horizontal;
//     u8 vertical_mask = 0x01;
//     u8 horizontal_mask = 0x01;
//     u8 badkey_iterator = 0;
//     u8 vertical_iterator= 0;
//     u8 horizontal_iterator = 0;
//     u16 key_id_value = g_LongKeyCode & 0x07ff;
//     memset(order_for_badkey, 0, 30);
//     if(key_id_value){
//         performance_status |= 0x20;
//         vertical = key_id_value >> 6;
//         horizontal = key_id_value & 0x3f;
//         for(vertical_iterator = 0; vertical_iterator < 5; vertical_iterator++)
//         {
//             if(vertical & (vertical_mask << vertical_iterator))
//             {
//                 for(horizontal_iterator = 0; horizontal_iterator < 6; horizontal_iterator++)
//                 {
//                     if(horizontal & (horizontal_mask << horizontal_iterator))
//                     {
//                         order_for_badkey[badkey_iterator] = keyboard_map[vertical_iterator][horizontal_iterator];
//                         badkey_iterator ++;
//                     }
//                 }
//             }
//         }

//     }
//     else
//         performance_status &= ~0x20;
//     number_for_badkey = badkey_iterator;
// }


void check_key_location() {
    u8 badkey_location, badkey_iterator = 0;
    u32 key_mask = 0x00000001;
    u32 key_id_value = g_LongKeyCode & 0x3fffffff;
    memset(order_for_badkey, 0, 30);
    if(key_id_value){
        performance_status |= 0x20;
        for (badkey_location = 0; badkey_location < 30; badkey_location++)
        {
            if(key_id_value & (key_mask << badkey_location))
            {
                order_for_badkey[badkey_iterator] = badkey_location + 1;
                badkey_iterator ++;
            } 
        }
    }
    else
        performance_status &= ~0x20;
    number_for_badkey = badkey_iterator;
}


void key_duty_task() {
    key_scan_task();
    check_key_location();
}


