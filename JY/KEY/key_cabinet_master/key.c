#include "key.h"
#include "led.h"
#include "utils.h"
#include "share.h"
#include "ble.h"
#include "string.h"


#define LOW     0
#define HIGH    1

static volatile u32 g_PreKey = 0;   //前次按键值 
static volatile u32 g_NowKey = 0;   //当前按键值 
static volatile u32 g_ShortKeyCode = 0;    //短按键,单次触发 
static volatile u32 g_LongKeyCode = 0;     //长按键,持续触发 


void key_io_init()
{
    // P02~P04
    quasi_bidirectional(0, 0x10);
		quasi_bidirectional(0, 0x08);
		quasi_bidirectional(0, 0x04);
    // P16~P17
    quasi_bidirectional(1, 0x80);
		quasi_bidirectional(1, 0x40);
    // P20~P27
    quasi_bidirectional(2, 0x80);
		quasi_bidirectional(2, 0x40);
		quasi_bidirectional(2, 0x20);
		quasi_bidirectional(2, 0x10);
		quasi_bidirectional(2, 0x08);
		quasi_bidirectional(2, 0x04);
		quasi_bidirectional(2, 0x02);
		quasi_bidirectional(2, 0x01);
    // P35
    quasi_bidirectional(3, 0x20);

    // 41~46
		quasi_bidirectional(4, 0x40);
		quasi_bidirectional(4, 0x20);
		quasi_bidirectional(4, 0x10);
		quasi_bidirectional(4, 0x08);
		quasi_bidirectional(4, 0x04);
		quasi_bidirectional(4, 0x02);
    // 51~52
    quasi_bidirectional(5, 0x04);
		quasi_bidirectional(5, 0x02);
    // 70~77
    quasi_bidirectional(7, 0x80);
		quasi_bidirectional(7, 0x40);
		quasi_bidirectional(7, 0x20);
		quasi_bidirectional(7, 0x10);
		quasi_bidirectional(7, 0x08);
		quasi_bidirectional(7, 0x04);
		quasi_bidirectional(7, 0x02);
		quasi_bidirectional(7, 0x01);
}


// 行列扫描的缺陷：当有多个键同时按下时，可能无法检测具体位置
// 不使用行列扫描，单独置行， 求列， 能否求出每个按键的位置？
u32 
IO_KeyScan_G() {
    u32 key = 0x00000003;
    // for(i = 30; i >= 1; i--)
    //     key = key << 1 | K(i);
    key = key << 1 | K30;
    key = key << 1 | K29;
    key = key << 1 | K28;
    key = key << 1 | K27;
    key = key << 1 | K26;
    key = key << 1 | K25;
    key = key << 1 | K24;
    key = key << 1 | K23;
    key = key << 1 | K22;
    key = key << 1 | K21;
    key = key << 1 | K20;
    key = key << 1 | K19;
    key = key << 1 | K18;
    key = key << 1 | K17;
    key = key << 1 | K16;
    key = key << 1 | K15;
    key = key << 1 | K14;    
    key = key << 1 | K13;
    key = key << 1 | K12;
    key = key << 1 | K11;
    key = key << 1 | K10;
    key = key << 1 | K9;
    key = key << 1 | K8;
    key = key << 1 | K7;
    key = key << 1 | K6;
    key = key << 1 | K5;
    key = key << 1 | K4;
    key = key << 1 | K3;
    key = key << 1 | K2;
    key = key << 1 | K1;
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
    // TX1_write2buff(g_LongKeyCode);
    // TX1_write2buff(g_LongKeyCode >> 8);
    // TX1_write2buff(g_LongKeyCode >> 16);
    // TX1_write2buff(g_LongKeyCode >> 24);
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
    u32 key_id_value;
    g_LongKeyCode = g_LongKeyCode & (~IO_KeyScan_G());
    key_id_value = g_LongKeyCode & 0x3fffffff;
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


