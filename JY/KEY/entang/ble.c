#include "ble.h"
#include "string.h"
#include "usart.h"
#include "task.h"
#include "config.h"
#include "stdio.h"
#include "share.h"
#include "sync.h"
#include "utils.h"
#include "entang.h"

#define ble_send_command(x)   PrintString1(x)
#define ble_recieve_data(x)   GetString1(x)
#define ble_send_data(x)      PrintString2(x)

static code unsigned char const BLE_IND_LINKED[] = "Connected\r\n";
static code unsigned char const BLE_IND_ADVERTISING[] = "Advertising\r\n";
static code unsigned char const BLE_IND_UNLINKED[] = "Disconnected\r\n";
static code unsigned char const BLE_IND_LINKED_TIME_OUT[] = "Timed Out\r\n";
static code unsigned char const BLE_IND_INIT[] = "Initialized\r\n";
static code unsigned char const BLE_IND_SUCCESS_CHANGE_BLE_NAME[] = "OK";
static code unsigned char const BLE_IND_CONNECT_BREAK[] = "Connection is broken";

static volatile u8 xdata TX1_Buffer[COM_TX1_Lenth];    //发送缓冲
static volatile u8 xdata RX1_Buffer[COM_RX1_Lenth];    //接收缓冲

u8 received_for_request = 0;
u8 received_for_ble_response = 0;


void init_uart()
{
    memset(RX1_Buffer, 0, COM_RX1_Lenth);
    COM1.RX_Cnt = 0;
    COM1.B_RX_OK = FALSE;
    received_for_request = 0;
    received_for_ble_response = 0;
}


void Delay2000ms()      //@24.000MHz
{
    unsigned char i, j, k;

    _nop_();
    _nop_();
    i = 183;
    j = 100;
    k = 225;
    do
    {
        do
        {
            while (--k);
        } while (--j);
    } while (--i);
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

// BLE DUTY: 
// 1. read buffer 
// 2. in case recieve new while reading, all new SBUF direct to RX1_Spare
// 3. when finish reading, RX1_Spare copy to RX1_Buffer, while new SBUF to index > RX1_Spare_Cnt
// 4. new handle buffer will start after prev copy is over
static void GetString1(u8 *received) {
    if (COM1.RX_Cnt) 
        memcpy(received, RX1_Buffer, COM1.RX_Cnt);
        received[COM1.RX_Cnt] = '\0';
    COM1.RX_Cnt = 0;
    COM1.B_RX_OK = FALSE;
    memset(RX1_Buffer, 0, COM_RX1_Lenth);
}

void PrintString1(u8 *puts)
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

void BLE_Enter_Adversting()
{
    // BT_RTS = 0;
    // sync_with_alert();
    // ble_send_command("AT+SNAM+MCCSCYCS39\r\n");
    // Delay50ms();
    // BT_RTS = 1;
    // Delay50ms();


    BT_RTS = 0;
    sync_with_alert();
    ble_send_command("AT+RSET\r\n");
    Delay50ms();
    BT_RTS = 1;
    init_uart();
}


void BLE_Init() {

    memset(TX1_Buffer, 0, COM_TX1_Lenth);
    memset(RX1_Buffer, 0, COM_RX1_Lenth);
    USART_Init(&COM1);

    S1_8bit();
    S1_BRT_UseTimer2();     // S1 BRT Use Timer2;

    S1_Int_Enable();        // 允许中断

    S1_RX_Enable();         // Rx 接收允许
 //   S1_USE_P30P31();
    S1_USE_P36P37();
    quasi_bidirectional(BLE_TX_Mx, BLE_TX_BIT);
    quasi_bidirectional(BLE_RX_Mx, BLE_RX_BIT);

    S1_TXD_RXD_OPEN();

    USART_Share_Timer2();


}

bool Check_Requset_for_Write_Password(u8 *received)
{
    u8 i, list_for_password[4];
    for(i = 0; i < 4; i++)
        list_for_password[i] = received[i+1];
    if(CheckComplement(received, 5, 8, received, 1, 4) && ChecksimpleCRC(received, 1, 8, received[9]) && CheckComplement(received, 10, 19, received, 0, 9))
    {
        Store_Password(list_for_password, 0, 3);
        return 0;
    }
    else
        return 1;
}

void Send_Response_for_Password(u8 *received)
{
    u8 send_data[20], i, password[4], key_in[16];
    memset(send_data, 0 , 20);
    if(!Check_Requset_for_Write_Password(received))
    { 
        send_data[0] = 0x02;
        // todo
        // 获取16字节的key
        for(i = 0; i < 4; i++)
            password[i] = received[i+1];
        ENTANG_A_init(password, key_in);
        for(i = 0; i < 16; i++)
           send_data[i + 1] = key_in[i];
        send_data[17] = CRC_Creat(send_data, 1, 8);
        send_data[18] = CRC_Creat(send_data, 9, 16);
        send_data[19] = (u8) (AND_Creat(send_data, 1, 16) ^ (AND_Creat(send_data, 1, 16) >> 8));
    }
    else
    {
        send_data[0] = 0x03;
        for(i = 1; i < 20; i++)
            send_data[i] = 0x00;
    }
    ble_send_data(send_data);
}


bool Check_Requset_for_Open_key_cabinet(u8 *received)
{
    u8 i, code_receive[16], decode[16];
    u32 timestamp_pre_high, timestamp_pre_low, timestamp_high, timestamp_low;
	  timestamp_pre_high = timestamp_pre_low = timestamp_high = timestamp_low = 0;
    for(i = 4; i < 20; i++)
        code_receive[i-4] = received[i];
    // 解密第4～19字节
    ENTANG_key_init();
    ENTANG_A_dec(code_receive, decode);
    
    // 64位无法存储、分开读取 
    for(i = 0; i < 4; i++)
        timestamp_high = timestamp_high << 8 | decode[i];
    for(i = 4; i < 8; i++)
        timestamp_low = timestamp_low << 8 | decode[i]; 
    
    timestamp_pre_high = Read_Timestamp_High();
    timestamp_pre_low = Read_Timestamp_Low();

    if((received[3] == received[1] ^ received[2]) && CheckComplement(decode, 0, 7, decode, 8, 15)
    && (timestamp_high > timestamp_pre_high || ((timestamp_high == timestamp_pre_high)  && (timestamp_low > timestamp_pre_low))))
    {
        Store_Timestamp(decode, 0, 7);
        return 0;
    }
    else
        return 1;

}


void Send_Response_for_Open_key_cabinet(u8 *received)
{
    u8 send_data[20] , i;
    memset(send_data, 0 , 20);
    send_data[0] = 0x05;
    if(!Check_Requset_for_Open_key_cabinet(received))
    { 
        send_data[1] = 0x00;
        // 开柜门
        candidate_driver_signal_recieving = YES;
        DRIVER = 1; // 开锁
    }
    else
        send_data[1] = 0x01;
    for(i = 2; i < 20; i++)
        send_data[i] = 0x00;
    ble_send_data(send_data);
}


// 黑科技指令：
// Request: 20字节
// 第0字节： 1字节， 命令ID， 0x80 - 0xFF 间随机值
// 第1字节： 1字节， 第0字节取反
// 第2字节： 1字节， 第0字节和第1字节异或
// 第3～18字节： 16字节， 8字节的Timestamp和8字节的Timestamp的取反）和第0字节异或
// 第19字节： 1字节： 第3～18字节的字节和的两个字节异或


bool Check_Requset_for_Key_request(u8 *received)        
{
    if(
        ((received[0] & 0x18) == (received[1] & 0x18)) 
        && (received[2] == (u8)(received[0] + received[1])) 
        && (received[3] ^ received[0]== ~(received[11] ^ received[0])) 
        && (received[4] ^ received[0]== ~(received[12] ^ received[0])) 
        && (received[5] ^ received[0]== ~(received[13] ^ received[0])) 
        && (received[6] ^ received[0]== ~(received[14] ^ received[0])) 
        && (received[7] ^ received[0]== ~(received[15] ^ received[0])) 
        && (received[8] ^ received[0]== ~(received[16] ^ received[0])) 
        && (received[9] ^ received[0]== ~(received[17] ^ received[0])) 
        && (received[10] ^ received[0]== ~(received[18] ^ received[0])) 
        && (received[19] == (u8) (AND_Creat(received, 3, 18) ^ (AND_Creat(received, 3, 18) >> 8)))
    )
    {
        return 0;
    }
    else
		{
        return 1;
		}
}



// Response: 20字节
// 第0字节：1字节，命令ID, 同Request指令第0字节
// 第1～16字节：16字节，生成的16字节的key 与 第0字节的异或
// 第17字节：1字节，第1～8字节的CRC校验
// 第18字节：1字节，第9～16字节的CRC校验
// 第19字节：1字节，第1～16字节的字节和的两个字节异或

void Send_Response_for_Key_request(u8 *received)
{
    u8 send_data[20] , i;
    u8 user_password[4], key_init[16];
    memset(send_data, 0 , 20);
    send_data[0] = received[0];
    if(!Check_Requset_for_Key_request(received))
    { 
        Read_Password(user_password, 0, 3);
        ENTANG_A_init(user_password, key_init);
        for(i = 1; i < 17; i++)
            send_data[i] = key_init[i-1] ^ send_data[0];
        send_data[17] = CRC_Creat(send_data, 1, 8);
        send_data[18] = CRC_Creat(send_data, 9, 16);
        send_data[19] = (u8) (AND_Creat(send_data, 1, 16) ^ (AND_Creat(send_data, 1, 16) >> 8));
    }
    else
    {
        send_data[1] = 0x01;
        for(i = 2; i < 20; i++)
            send_data[i] = 0x00;
    }
    ble_send_data(send_data);
}


void ble_process_recieved() {
    unsigned char received[32];
    ble_recieve_data(received);
    if (strcmp(received, BLE_IND_LINKED) == 0) {
        // 5s没发数据、断开
        reset_all_counters();
        candidate_time_out_signal_recieving = YES;
        reset_counter_for_ble();
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
    } else if(strcmp(received, BLE_IND_SUCCESS_CHANGE_BLE_NAME) == 0){
        reset_all_counters();
        candidate_time_out_signal_recieving = NO;
        BLE_Enter_Adversting(); 
    } 
    // else {
    //     // 错误数据直接断开进入广播模式
    //     reset_all_counters();
    //     candidate_time_out_signal_recieving = NO;
    //     BLE_Enter_Adversting();
    // }
}



void request_process_recieved()
{
    unsigned char received[20];
    ble_recieve_data(received);
    // 判断开关
    if((received[0] == 0x01) && !CODE){
        candidate_time_out_signal_recieving = NO;
        reset_all_counters();
        Send_Response_for_Password(received);
    }else if(received[0] == 0x04){
        candidate_time_out_signal_recieving = NO;
        reset_all_counters();
        Send_Response_for_Open_key_cabinet(received);
    }else if(received[0] >= 0x80 && received[0] <= 0xFF ){
        candidate_time_out_signal_recieving = NO;
        reset_all_counters();
        Send_Response_for_Key_request(received);
    } else {
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
        if(COM1.B_RX_OK == 0)
        {
            if(!received_for_ble_response && !received_for_request)
            {
                if((SBUF >= 65 && SBUF <= 90) || (SBUF >= 97 && SBUF <= 122))
                    received_for_ble_response = 1;
                else
                    received_for_request = 1;
            }
            if(received_for_ble_response)
            {
                RX1_Buffer[COM1.RX_Cnt++] = SBUF;
                if (SBUF == '\n') {
                    COM1.B_RX_OK = 1;
                    received_for_ble_response = 0;
                    INT_PROC |= BLE_DUTY;
                }
                else if(RX1_Buffer[1] == 'K' && RX1_Buffer[0] == 'O')
                {
                    COM1.RX_Cnt = 0;
                    received_for_ble_response = 0;
                }
                if (COM1.RX_Cnt >= COM_RX1_Lenth)
                {    
                    COM1.RX_Cnt = 0;
                    received_for_ble_response = 0;
                }
            }
            if(received_for_request)
            {
                RX1_Buffer[COM1.RX_Cnt++] = SBUF;
                if (COM1.RX_Cnt == 20) {
                    COM1.B_RX_OK = 1;
                    INT_PROC |= REQUEST_DUTY;
                    received_for_request = 0;
                }
            }
            COM1.RX_TimeOut = TimeOutSet1;
        } 
        RI = 0;
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