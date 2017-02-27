#include "ble.h"
#include "string.h"
#include "usart.h"
#include "task.h"
#include "config.h"
#include "stdio.h"
#include "share.h"
#include "sync.h"
#include "utils.h"
#include "key.h"
#include "debug.h"
#define error_code 0x00

#define ble_send_command(x)   PrintString1(x)
#define ble_recieve_data(x)   GetString1(x)
#define ble_send_data(x)      PrintString2(x)

static code unsigned char const BLE_IND_LINKED[] = "Connected\r\n";
static code unsigned char const BLE_IND_ADVERTISING[] = "Advertising\r\n";
static code unsigned char const BLE_IND_UNLINKED[] = "Disconnected\r\n";
static code unsigned char const BLE_IND_LINKED_TIME_OUT[] = "Timed Out\r\n";
static code unsigned char const BLE_IND_INIT[] = "Initialized\r\n";
static code unsigned char const BLE_IND_PASSWORD[] = "VerificateIdentity\r\n";
static code unsigned char const BLE_IND_UNLOCKED[] = "Please_openthedoor\r\n";
static code unsigned char const BLE_IND_SUCCESS_CHANGE_BLE_NAME[] = "OK";
static code unsigned char const BLE_IND_CONNECT_BREAK[] = "Connection is broken";

static volatile u8 xdata TX1_Buffer[COM_TX1_Lenth];    //发送缓冲
static volatile u8 xdata RX1_Buffer[COM_RX1_Lenth];    //接收缓冲


void init_uart()
{
    memset(RX1_Buffer, 0, COM_RX1_Lenth);
    COM1.RX_Cnt = 0;
    COM1.B_RX_OK = FALSE;
}

void Delay50ms()        //@24.000MHz
{
    unsigned char i, j, k;

    _nop_();
    _nop_();
    i = 5;
    j = 144;
    k = 71;
    do
    {
        do
        {
            while (--k);
        } while (--j);
    } while (--i);
}

void TX1_write2buff(const u8 dat) 
{
    TX1_Buffer[COM1.TX_write] = dat;        // 装发送缓冲
    if(++COM1.TX_write >= COM_TX1_Lenth)    
        COM1.TX_write = 0;

    if(COM1.B_TX_busy == 0)                 // 空闲
    {  
        COM1.B_TX_busy = 1;                 // 标志忙
        TI = 1;                             // 触发发送中断
    }
}



static void GetString1(u8 *received) {
    if (COM1.RX_Cnt) 
        memcpy(received, RX1_Buffer, COM1.RX_Cnt);
    received[COM1.RX_Cnt] = '\0';
    memset(RX1_Buffer, 0, COM_RX1_Lenth);
    COM1.RX_Cnt = 0;
    COM1.B_RX_OK = FALSE;
}

static void PrintString1(u8 *puts)
{
    for (; *puts != 0;  puts++)  TX1_write2buff(*puts);     //遇到停止符0结束
}

static void PrintString2(u8 *puts)
{
    u8 i;
    for(i = 0 ; i < 20; i++)
         TX1_write2buff(puts[i]);
     Delay50ms();
}
void BLE_Init() {

    memset(TX1_Buffer, 0, COM_TX1_Lenth);
    memset(RX1_Buffer, 0, COM_RX1_Lenth);
    USART_Init(&COM1);

    S1_8bit();
    S1_BRT_UseTimer2();     // S1 BRT Use Timer2;

    S1_Int_Enable();        // 允许中断

    S1_RX_Enable();         // Rx 接收允许
    S1_USE_P36P37();
    quasi_bidirectional(BLE_TX_Mx, BLE_TX_BIT);
    quasi_bidirectional(BLE_RX_Mx, BLE_RX_BIT);

    S1_TXD_RXD_OPEN();

    USART_Share_Timer2();
}

void BLE_Enter_Adversting()
{
    BT_RTS = 0;
    sync_with_alert();
    ble_send_command("AT+RSET\r\n");
    Delay50ms();
    BT_RTS = 1;
    init_uart();
}


void BLE_ChangeBLEName(u8 *blename)
{
    u8 i;
    u8 cmd[22] = {'A','T','+','S','N','A','M','+', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,'\r','\n'};
    for(i = 8; i <= 19; i++)
        cmd[i] = blename[i-8];
    BT_RTS = 0;
    sync_with_alert();
    for(i = 0; i < 22; i++)
    {
        TX1_write2buff(cmd[i]);
    } 
    Delay50ms();
    BT_RTS = 1;
}

bool Check_Requset_for_Read_counter_for_operation(u8 *received)
{
    u8 i, timestamp[6];
    for(i = 0; i < 6; i++)
        timestamp[i] = ~received[i+1];
    if((received[7] == ~ received[0]) && ChecksimpleCRC(received, 0, 7, received[8]) && 
        CheckExclusive_Or(timestamp, 0, 5, received, 0, 5, received, 9, 14) && ChecksimpleCRC(received, 7, 14, received[15]) && CheckAnd(received, 0, 7, ~received[16], ~received[17]) && CheckAnd(received, 8, 15, ~received[18], ~received[19]))
    {
        return 0;
    }
    else
        return 1;
}

void Send_Response_for_Read_counter_for_operation(u8 *received)
{
    u8 send_data[20], i, timestamp[6];
    u32 couter_for_operation;
    memset(send_data, 0 , 20);
    send_data[0] = received[0] + 1;
    if(!Check_Requset_for_Read_counter_for_operation(received))
    { 
        send_data[1] = 0x00;
        // todo
        if(received[0] == 0x00){ 
            couter_for_operation = Read_Counter_for_Open_key_cabinet();
        }else if(received[0] == 0x02){
            couter_for_operation = Read_Counter_for_Open_flood_door();
        }else if(received[0] == 0x04){
            couter_for_operation = Read_Counter_for_Read_A_and_B();
        }else if(received[0] == 0x06){
            couter_for_operation = Read_Counter_for_Write_A();
        }else if(received[0] == 0x16){
            couter_for_operation = Read_Counter_for_change_BLEname();
        }
        // todo
        // 硬件状态
        // performance_status 
        send_data[2] = performance_status;
        // 3~5 位取操作数后三位再取反
        // 高位在前
        for(i = 3; i <= 5; i++)
            send_data[i] = ~(couter_for_operation >> (8 * (5-i)));
        // 6~11字节取Timestamp
        for(i = 0; i < 6; i++)
            timestamp[i] = ~received[i+1];
        for(i = 6; i < 12; i++)
            send_data[i] = ~timestamp[i-6];
        // 12~15
        Read_A(send_data, 12, 15);
        Exclusive_Or_Creat(send_data, 12, 15, timestamp, 2, 5, send_data, 12, 15);
        Complement_Creat(send_data, 12, 15, send_data, 12, 15);
        // 16~19
        send_data[16] = CRC_Creat(send_data, 0, 7);
        send_data[17] = CRC_Creat(send_data, 8, 15);
        send_data[18] = (u8) (AND_Creat(send_data, 0, 17) >> 8);
        send_data[19] = (u8) AND_Creat(send_data, 0, 17);
    }
    else
    {
        send_data[1] = 0x01;
        for(i = 2; i < 17; i++)
            send_data[i] = 0x00;
        send_data[18] = send_data[19] = error_code;
    }
    ble_send_data(send_data);
}

bool Check_Requset_for_Open_floodgate(u8 *received)
{
    u8 i, list_for_Open_flood_door[4], counter_for_B[4], order_over_creat_for_B[4], timestamp[6];
    u32 counter_for_Open_flood_door;
    // 取操作次数
    counter_for_Open_flood_door = Read_Counter_for_Open_flood_door();
    list_for_Open_flood_door[0] = (u8) (counter_for_Open_flood_door >> 24);
    list_for_Open_flood_door[1] = (u8) (counter_for_Open_flood_door >> 16);
    list_for_Open_flood_door[2] = (u8) (counter_for_Open_flood_door >> 8);
    list_for_Open_flood_door[3] = (u8) counter_for_Open_flood_door;
    // 取B
    Read_B(counter_for_B, 0, 3);
    Order_Over_Creat(counter_for_B, 0, 3, order_over_creat_for_B, 0, 3);
    for(i = 0; i < 6; i++)
        timestamp[i] = ~ received[i+10];



    if(CheckComplement(list_for_Open_flood_door, 1, 3, received, 1, 3) && CheckExclusive_Or(order_over_creat_for_B, 0, 3, received, 0, 3,  received, 4, 7) && CheckAnd(counter_for_B, 0, 3, ~received[8], ~received[9]) && ChecksimpleCRC(received, 0, 7, received[16]) && ChecksimpleCRC(received, 8, 15, received[17]) && CheckAnd(received, 0, 17, received[18], received[19]))
    {
        Store_Counter_for_Open_flood_door(counter_for_Open_flood_door+1);
        return 0;
    }
    else
    {
        return 1;
    }
}
void Send_Response_for_Open_floodgate(u8 *received)
{
    u8 send_data[20] , i , timestamp[6], timestamp_complement[6];
    memset(send_data, 0 , 20);
    send_data[0] = 0x09;
    if(!Check_Requset_for_Open_floodgate(received))
    { 
        send_data[1] = 0x00;
        // todo
        // 2~7字节取Timestamp
        for(i = 0; i < 6; i++)
            timestamp[i] = ~ received[i+10];
        for(i = 2; i <= 7; i++ )
            send_data[i] = timestamp[i-2];
        send_data[8] = CRC_Creat(send_data, 0, 7);
        send_data[9] = ~ send_data[0];
        send_data[10] = (u8) (~ (AND_Creat(send_data, 0, 7) >> 8));
        send_data[11] = (u8) (~ AND_Creat(send_data, 0, 7));
        
        send_data[12] = (u8) (~ (AND_Creat(send_data, 8, 11) >> 8));
        send_data[13] = (u8) (~ AND_Creat(send_data, 8, 11));
        // 14 ~ 19
        Complement_Creat(send_data, 2, 7, timestamp_complement, 0, 5);
        Exclusive_Or_Creat(timestamp_complement, 0, 5, send_data, 8, 13, send_data, 14, 19);
        ble_send_data(send_data);
        // 5S内开锁信号
        // 开锁(持续一秒的高电平)
        // candidate_time_out_signal_recieving = NO;
        // reset_all_counters();
        // candidate_driver_signal_recieving = YES;
        // DRIVER = 1;
    }
    else
    {
        send_data[1] = 0x01;
        for(i = 2; i < 17; i++)
            send_data[i] = 0x00;
        send_data[18] = send_data[19] = error_code;
        ble_send_data(send_data);
    }

}

bool Check_Requset_for_Open_key_cabinet(u8 *received)
{
    u8 timestamp[6] , i, list_for_Open_key_cabinet[4], counter_for_B[4], order_over_creat_for_B[4];
    u32 counter_for_Open_key_cabinet;
    // 取操作次数
     counter_for_Open_key_cabinet = Read_Counter_for_Open_key_cabinet();
     list_for_Open_key_cabinet[0] = counter_for_Open_key_cabinet >> 24;
     list_for_Open_key_cabinet[1] = counter_for_Open_key_cabinet >> 16;
     list_for_Open_key_cabinet[2] = counter_for_Open_key_cabinet >> 8;
     list_for_Open_key_cabinet[3] = counter_for_Open_key_cabinet ;
    // 取B
    Read_B(counter_for_B, 0, 3);
    Order_Over_Creat(counter_for_B, 0, 3, order_over_creat_for_B, 0, 3);
    for(i = 0; i < 6; i++)
        timestamp[i] = ~ received[i+10];
    // 第8字节柜门编号
    key_cabinet_number = (~received[8]) + 1;
    if(CheckComplement(list_for_Open_key_cabinet, 1, 3, received, 1, 3) && CheckExclusive_Or(order_over_creat_for_B, 0, 3, received, 0, 3,  received, 4, 7) && (received[9] == ((~ received[8]) ^ received[3])) && ChecksimpleCRC(received, 0, 7, received[16]) && ChecksimpleCRC(received, 8, 15, received[17]) && CheckAnd(received, 0, 17, received[18], received[19]))
    {
        Store_Counter_for_Open_key_cabinet(counter_for_Open_key_cabinet + 1);  
        return 0;
    }
    else
        return 1;

}
void Send_Response_for_Open_key_cabinet(u8 *received)
{
    u8 send_data[20] , i , timestamp[6], timestamp_complement[6];
    memset(send_data, 0 , 20);
    send_data[0] = 0x0C;
    if(!Check_Requset_for_Open_key_cabinet(received))
    { 
        send_data[1] = 0x00;
        // todo
        // 2~7字节取Timestamp
        for(i = 0; i < 6; i++)
        timestamp[i] = ~ received[i+10];
        for(i = 2; i <= 7; i++ )
            send_data[i] = timestamp[i-2];
        send_data[8] = CRC_Creat(send_data, 0, 7);
        send_data[9] = (~ send_data[0]) ^ (~ received[8]);
        send_data[10] = (u8) (~ (AND_Creat(send_data, 0, 7) >> 8));
        send_data[11] = (u8) (~ AND_Creat(send_data, 0, 7));
        
        send_data[12] = (u8) (~ (AND_Creat(send_data, 8, 11) >> 8));
        send_data[13] = (u8) (~ AND_Creat(send_data, 8, 11));
        // 14 ~ 19
        Complement_Creat(send_data, 2, 7, timestamp_complement, 0, 5);
        Exclusive_Or_Creat(timestamp_complement, 0, 5, send_data, 8, 13, send_data, 14, 19);
        ble_send_data(send_data);
        // 开柜门
        // 开锁(持续0.3秒的高电平)
        if(key_cabinet_number == 29 || key_cabinet_number == 30)
        {
            candidate_driver_signal_recieving = YES;
            driver_high_pin(key_cabinet_number);
        }
        else
        {
            putchar(key_cabinet_number);
        }
    }
    else
    {
        send_data[1] = 0x01;
        for(i = 2; i < 17; i++)
            send_data[i] = 0x00;
        send_data[18] = send_data[19] = error_code;
        ble_send_data(send_data);
    }

    
}


bool Check_Requset_for_Read_A_and_B(u8 *received)
{
    u8 i, list_for_Read_A_and_B[4], timestamp[6];
    u32 counter_for_Read_A_and_B;
    // 取操作次数
    counter_for_Read_A_and_B = Read_Counter_for_Read_A_and_B();
    list_for_Read_A_and_B[0] = counter_for_Read_A_and_B >> 24;
    list_for_Read_A_and_B[1] = counter_for_Read_A_and_B >> 16;
    list_for_Read_A_and_B[2] = counter_for_Read_A_and_B >> 8;
    list_for_Read_A_and_B[3] = counter_for_Read_A_and_B;
    for(i = 0; i < 6; i++)
        timestamp[i] = ~ received[i+4];
    if(CheckComplement(list_for_Read_A_and_B, 1, 3, received, 1, 3) && CheckExclusive_Or(timestamp, 0, 5, received, 0, 5, received, 10, 15) && ChecksimpleCRC(received, 0, 7, received[16]) && ChecksimpleCRC(received, 8, 15, received[17]) && CheckAnd(received, 0, 17, received[18], received[19]))
    {
        // Store_Timestamp(timestamp, 0, 5);
        Store_Counter_for_Read_A_and_B(counter_for_Read_A_and_B + 1);
        return 0;
    }
    else
        return 1;
}
void Send_Response_for_Read_A_and_B(u8 *received)
{
    u8 send_data[20], i, timestamp[6];
    memset(send_data, 0 , 20);
    send_data[0] = 0x0F;
    if(!Check_Requset_for_Read_A_and_B(received))
    { 
        send_data[1] = 0x00;
        // todo
        // 2~7字节取Timestamp
        for(i = 0; i < 6; i++)
            timestamp[i] = ~ received[i+4];
        for(i = 2; i <= 7; i++ )
            send_data[i] = timestamp[i-2];
        Read_A(send_data, 8, 11);
        Read_B(send_data, 12, 15);
        // 8 ~ 11
        Exclusive_Or_Creat(send_data, 8, 11, send_data, 4, 7, send_data, 8, 11);
        Complement_Creat(send_data, 8, 11, send_data, 8, 11);
        // 12 ~ 15
        Exclusive_Or_Creat(send_data, 12, 15, send_data, 0, 3, send_data, 12, 15);
        Complement_Creat(send_data, 12, 15, send_data, 12, 15);
        send_data[16] = CRC_Creat(send_data, 0, 7);
        send_data[17] = CRC_Creat(send_data, 8, 15);
        send_data[18] = (u8) (~ (AND_Creat(send_data, 0, 17) >> 8)) ;
        send_data[19] = (u8) (~ AND_Creat(send_data, 0, 17));
    }
    else
    {
        send_data[1] = 0x01;
        for(i = 2; i < 17; i++)
            send_data[i] = 0x00;
        send_data[18] = send_data[19] = error_code;
    }
     ble_send_data(send_data);
    
}
bool Check_Requset_for_Write_A(u8 *received)
{
    u8 i, list_for_Write_A[4], counter_for_A[4], timestamp[6];
    u32 counter_for_Write_A;
    // 取操作次数
    counter_for_Write_A = Read_Counter_for_Write_A();
    list_for_Write_A[0] = counter_for_Write_A >> 24;
    list_for_Write_A[1] = counter_for_Write_A >> 16;
    list_for_Write_A[2] = counter_for_Write_A >> 8;
    list_for_Write_A[3] = counter_for_Write_A ;
    for(i = 0; i < 6; i++)
        timestamp[i] = ~ received[i+4];
    Complement_Creat(received, 10, 13, counter_for_A, 0, 3);
    if(CheckComplement(list_for_Write_A, 1, 3, received, 1, 3) && CheckAnd(received, 0, 13, received[14], received[15]) && ChecksimpleCRC(received, 0, 7, received[16]) && ChecksimpleCRC(received, 8, 15, received[17]) && CheckAnd(received, 0, 17, received[18], received[19]))
    {
        Store_A(counter_for_A, 0, 3);
        Store_Counter_for_Operate_A_B_and_Blename((counter_for_Write_A + 1), 1);
        return 0;
    }
    else
        return 1;
}
void Send_Response_for_Write_A(u8 *received)
{
    u8 send_data[20], i, counter_for_A[4], timestamp[6];
    memset(send_data, 0 , 20);
    send_data[0] = 0x11;
    if(!Check_Requset_for_Write_A(received))
    { 
        send_data[1] = 0x00;
        // todo
        for(i = 0; i < 6; i++)
           timestamp[i] = ~ received[i+4];
        // 2~7字节取Timestamp
        for(i = 2; i <= 7; i++ )
            send_data[i] = timestamp[i-2];
        // 
        Complement_Creat(received, 10, 13, counter_for_A, 0, 3);
        // 8 ~ 11
        Exclusive_Or_Creat(counter_for_A, 0, 3, send_data, 4, 7, send_data, 8, 11);
        Complement_Creat(send_data, 8, 11, send_data, 8, 11);
        // 12 ~ 15
        Exclusive_Or_Creat(counter_for_A, 0, 3, send_data, 0, 3, send_data, 12, 15);
        Complement_Creat(send_data, 12, 15, send_data, 12, 15);
        send_data[16] = CRC_Creat(send_data, 0, 7);
        send_data[17] = CRC_Creat(send_data, 8, 15);
        send_data[18] = (u8) ((~ AND_Creat(send_data, 0, 17)) >> 8);
        send_data[19] = (u8) (~ AND_Creat(send_data, 0, 17));
    }
    else
    {
        send_data[1] = 0x01;
        for(i = 2; i < 17; i++)
            send_data[i] = 0x00;
        send_data[18] = send_data[19] = error_code;
    }
    ble_send_data(send_data);
    
}
bool Check_Requset_for_ChangeBLEname(u8 *received)
{
    u8 list_for_ChangeBLEname[4];
    u32 counter_for_change_BLEname;
    counter_for_change_BLEname = Read_Counter_for_change_BLEname();
    list_for_ChangeBLEname[0] = counter_for_change_BLEname >> 24;
    list_for_ChangeBLEname[1] = counter_for_change_BLEname >> 16;
    list_for_ChangeBLEname[2] = counter_for_change_BLEname >> 8;
    list_for_ChangeBLEname[3] = counter_for_change_BLEname;
    if(CheckComplement(list_for_ChangeBLEname, 1, 3, received, 1, 3) && ChecksimpleCRC(received, 0, 7, received[16]) && ChecksimpleCRC(received, 8, 15, received[17]) && CheckAnd(received, 0, 17, received[18], received[19]))
    {
        Store_Counter_for_Operate_A_B_and_Blename((counter_for_change_BLEname + 1), 2);
        return 0;
    }
    else
        return 1;
    
}
void Send_Response_for_ChangeBLEname(u8 *received)
{
    u8 send_data[20], i, blename[12];
    memset(send_data, 0 , 20);
    send_data[0] = 0x13;
    if(!Check_Requset_for_ChangeBLEname(received))
    {    
        send_data[1] = 0x00;
        // 2~7字节：电话号码
        send_data[2] = 0x00;
        send_data[3] = 0x40;
        send_data[4] = 0x00;
        send_data[5] = 0x03;
        send_data[6] = 0x81;
        send_data[7] = 0x36;
        Read_A(send_data, 8, 11);
        // 8 ~ 11
        Exclusive_Or_Creat(send_data, 8, 11, send_data, 4, 7, send_data, 8, 11);
        Complement_Creat(send_data, 8, 11, send_data, 8, 11);
        // 12 ~ 15
        Read_A(send_data, 12, 15);
        Exclusive_Or_Creat(send_data, 12, 15, send_data, 0, 3, send_data, 12, 15);
        Complement_Creat(send_data, 12, 15, send_data, 12, 15);
        send_data[16] = CRC_Creat(send_data, 0, 7);
        send_data[17] = CRC_Creat(send_data, 8, 15);
        send_data[18] = (u8) ((~ AND_Creat(send_data, 0, 17)) >> 8);
        send_data[19] = (u8) (~ AND_Creat(send_data, 0, 17));
        for(i = 4; i <= 15; i++)
            blename[i - 4] = received[i];
        BLE_ChangeBLEName(blename);
        Delay50ms();

        ble_send_data(send_data);
        Delay50ms();

        BLE_Enter_Adversting();
    }
    else
    {
        send_data[1] = 0x01;
        for(i = 2; i < 17; i++)
            send_data[i] = 0x00;
        send_data[18] = send_data[19] = error_code;
        ble_send_data(send_data);
    }
    
}


u32 Read_performance_key_cabinet()
{
    u8 i, iterator = 0;
    u32 status_for_key_cabinet = 0;
    if(number_for_badkey)
    {
        for(i = 0; i < 32; i++)
        {
            if((iterator < number_for_badkey) && (i == (order_for_badkey[iterator] - 1)))
            {
                status_for_key_cabinet = status_for_key_cabinet << 1 | 1;
                iterator ++;
            }
            else 
                status_for_key_cabinet = status_for_key_cabinet << 1 | 0;
        }
    }
    return status_for_key_cabinet;
}


bool Check_Requset_for_Read_status_for_key_cabinet(u8 *received)
{
    u8 i, timestamp[6];
    for(i = 0; i < 6; i++)
        timestamp[i] = ~ received[i+1];
    if((received[7] == ~ received[0]) && ChecksimpleCRC(received, 0, 7, received[8]) && 
        CheckExclusive_Or(timestamp, 0, 5, received, 0, 5, received, 9, 14) && ChecksimpleCRC(received, 7, 14, received[15]) && CheckAnd(received, 0, 7, ~received[16], ~received[17]) && CheckAnd(received, 8, 15, ~received[18], ~received[19]))
    {
        return 0;
    }
    else
        return 1;   
}

void Send_Response_for_Read_status_for_key_cabinet(u8 *received)
{
    u8 send_data[20], i;
    u32 status_for_key_cabinet = Read_performance_key_cabinet();
    memset(send_data, 0 , 20);
    send_data[0] = 0x15;
    if(!Check_Requset_for_Read_status_for_key_cabinet(received))
    { 
        send_data[1] = 0x00;
        // todo
        // 开柜门次数最后一位取反;
        send_data[2] = ~((u8)(Read_Counter_for_Open_key_cabinet()));

        // 柜门状态
        // for(code_key_cabinet = 3; code_key_cabinet < 15; code_key_cabinet++)
        //     for(i = 0; i < 8; i++)
        //         send_data[code_key_cabinet] =send_data[code_key_cabinet] << 1 | Read_status_key_cabinet(code_key_cabinet);
        send_data[3] = status_for_key_cabinet >> 24;
        send_data[4] = status_for_key_cabinet >> 16;
        send_data[5] = status_for_key_cabinet >> 8;
        send_data[6] = status_for_key_cabinet;
        send_data[12] = send_data[13] = send_data[14] = send_data[15] = send_data[7] = send_data[8] = send_data[9] = send_data[10] = send_data[11] = 0x00;
        send_data[16] = CRC_Creat(send_data, 0, 7);
        send_data[17] = CRC_Creat(send_data, 8, 15);
        send_data[18] = (u8) (AND_Creat(send_data, 0, 17) >> 8);
        send_data[19] = (u8) AND_Creat(send_data, 0, 17);
    }
    else
    {
        send_data[1] = 0x01;
        for(i = 2; i < 17; i++)
            send_data[i] = 0x00;
        send_data[18] = send_data[19] = error_code;
    }
    ble_send_data(send_data); 
}


bool Check_Requset_for_Read_performance_for_key_cabinet(u8 *received)
{
    u8 i, timestamp[6];
    for(i = 0; i < 6; i++)
        timestamp[i] = ~ received[i+1];
    if((received[7] == ~ received[0]) && ChecksimpleCRC(received, 0, 7, received[8]) && 
        CheckExclusive_Or(timestamp, 0, 5, received, 0, 5, received, 9, 14) && ChecksimpleCRC(received, 7, 14, received[15]) && CheckAnd(received, 0, 7, ~received[16], ~received[17]) && CheckAnd(received, 8, 15, ~received[18], ~received[19]))
    {
        return 0;
    }
    else
        return 1;   
}

void Send_Response_for_Read_performance_for_key_cabinet(u8 *received)
{
    u8 send_data[20], i;
    u32 performance_for_key_cabinet = ~ Read_performance_key_cabinet();
    memset(send_data, 0 , 20);
    send_data[0] = 0x19;
    if(!Check_Requset_for_Read_performance_for_key_cabinet(received))
    { 
        send_data[1] = 0x00;
        send_data[2] = ~((u8)(Read_Counter_for_Open_key_cabinet()));
        // for(code_key_cabinet = 3; code_key_cabinet < 15; code_key_cabinet++)
        //     for(i = 0; i < 8; i++)
        //         send_data[code_key_cabinet] =send_data[code_key_cabinet] << 1 | Read_status_key_cabinet(code_key_cabinet);
        send_data[3] = performance_for_key_cabinet >> 24;
        send_data[4] = performance_for_key_cabinet >> 16;
        send_data[5] = performance_for_key_cabinet >> 8;
        send_data[6] = performance_for_key_cabinet;
        send_data[12] = send_data[13] = send_data[14] = send_data[15] = send_data[7] = send_data[8] = send_data[9] = send_data[10] = send_data[11] = 0xff;

        send_data[16] = CRC_Creat(send_data, 0, 7);
        send_data[17] = CRC_Creat(send_data, 8, 15);
        send_data[18] = (u8) (AND_Creat(send_data, 0, 17) >> 8);
        send_data[19] = (u8) AND_Creat(send_data, 0, 17);
    }
    else
    {
        send_data[1] = 0x01;
        for(i = 2; i < 17; i++)
            send_data[i] = 0x00;
        send_data[18] = send_data[19] = error_code;
    }
    ble_send_data(send_data); 
}

void handle_uart_data(){
    unsigned char received[32];
    ble_recieve_data(received);
    if((received[0] >= 65 && received[0] <= 90) || (received[0] >= 97 && received[0] <= 122)){
        request_process_recieved(received);
    } else {
        ble_process_recieved(received);
    }
}

void ble_process_recieved(char *received) {
    if (strcmp(received, BLE_IND_LINKED) == 0) {
        // 5s没发数据、断开
        reset_counter_for_ble();
        reset_all_counters();
        candidate_time_out_signal_recieving = YES;
        // candidate_time_out_signal_recieving = NO;
    } else if (strcmp(received, BLE_IND_ADVERTISING) == 0) {
        candidate_time_out_signal_recieving = NO;
        _nop_();
    } else if (strcmp(received, BLE_IND_LINKED_TIME_OUT) == 0) {
        candidate_time_out_signal_recieving = NO;
        BLE_Enter_Adversting();
    } else if (strcmp(received, BLE_IND_UNLINKED) == 0) {
        candidate_time_out_signal_recieving = NO;
        _nop_();
    } else if (strcmp(received, BLE_IND_INIT) == 0) {
        candidate_time_out_signal_recieving = NO;
        _nop_();
    } else if (strcmp(received, BLE_IND_PASSWORD) == 0) {
        // 5s内验证通过、不再断开
        candidate_time_out_signal_recieving = NO;
        reset_all_counters();
    } else if (strcmp(received, BLE_IND_UNLOCKED) == 0) {
        // 5S内开锁信号
        // 开锁(持续一秒的高电平)
       // candidate_time_out_signal_recieving = NO;
       // reset_all_counters();
       // candidate_driver_signal_recieving = YES;
       // DRIVER = 1; // 开锁
    } else if(strcmp(received, BLE_IND_SUCCESS_CHANGE_BLE_NAME) == 0){
        reset_all_counters();
        candidate_time_out_signal_recieving = NO;
        BLE_Enter_Adversting(); 
    } else {
        // 错误数据直接断开进入广播模式
        reset_all_counters();
        candidate_time_out_signal_recieving = NO;
        BLE_Enter_Adversting();
    }
}

void request_process_recieved(char *received)
{
    if(received[0] == 0x00 || received[0] == 0x02 || received[0] == 0x04 || received[0] == 0x06 || received[0] == 0x16){
        candidate_time_out_signal_recieving = NO;
        reset_all_counters();
        Send_Response_for_Read_counter_for_operation(received);
    }else if(received[0] == 0x08){
        candidate_time_out_signal_recieving = NO;
        reset_all_counters();
        Send_Response_for_Open_floodgate(received);
    }else if(received[0] == 0x0B){
        candidate_time_out_signal_recieving = NO;
        reset_all_counters();
        Send_Response_for_Open_key_cabinet(received);
    }else if(received[0] == 0x0E){
        candidate_time_out_signal_recieving = NO;
        reset_all_counters();
        Send_Response_for_Read_A_and_B(received);
    }else if(received[0] == 0x10){
        candidate_time_out_signal_recieving = NO;
        reset_all_counters();
        Send_Response_for_Write_A(received);
    }else if(received[0] == 0x12){
        candidate_time_out_signal_recieving = NO;
        reset_all_counters();   
        Send_Response_for_ChangeBLEname(received);
    }else if(received[0] == 0x14){
        candidate_time_out_signal_recieving = NO;
        reset_all_counters();
        Send_Response_for_Read_status_for_key_cabinet(received);
    }else if(received[0] == 0x18){
        candidate_time_out_signal_recieving = NO;
        reset_all_counters();
        Send_Response_for_Read_performance_for_key_cabinet(received);
    }else{
        // 错误数据
        reset_all_counters();
        candidate_time_out_signal_recieving = NO;
        BLE_Enter_Adversting();
    }

}
// Handle usart trigger: receiving "\r\n"
void UART1_int (void) interrupt UART1_VECTOR
{
    if(RI)
    {
        RI = 0;
        if(COM1.B_RX_OK == 0)
        {
            if (COM1.RX_Cnt >= COM_RX1_Lenth)
                COM1.RX_Cnt = 0;
            RX1_Buffer[COM1.RX_Cnt++] = SBUF;
            COM1.RX_TimeOut = TimeOutSet1;
        }

    }

    if(TI)
    {
        TI = 0;
        if(COM1.TX_read != COM1.TX_write)
        {
            SBUF = TX1_Buffer[COM1.TX_read];
            if(++COM1.TX_read >= COM_TX1_Lenth)     
                COM1.TX_read = 0;
        }
        else    
            COM1.B_TX_busy = 0;
    }
}
