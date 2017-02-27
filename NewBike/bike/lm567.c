 #include "lm567.h"
 #include <stdio.h> 
 #include "string.h" 
 #include "usart.h" 
 #include "task.h" 

 #include "sync.h" 
 #include "utils.h" 
 #include "key.h" 
 #include "power.h" 
 #include "beep.h" 

volatile send_id uart_id = open_success;
static volatile u8 xdata TX1_Buffer[COM_TX1_Lenth];    //发送缓冲
static volatile u8 xdata RX1_Buffer[COM_RX1_Lenth];    //接收缓冲

#define CMD_BUF_SIZE 20
static char  xdata command[CMD_BUF_SIZE];
#define RESP_BUF_SIZE 64
static char  xdata cached_response_line[RESP_BUF_SIZE];

// static xdata string uart_string[]={
//     "OK\r\n",
//     "+CCSHU:ERROR\r\n",
//     "+LOCK:1\r\nOK\r\n",
//     "+LOCK:0\r\nERROR\r\n",
//     "",
//     ""
// };



void TX1_write2buff(char dat)  
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

void TX_str2buff(char *str, u8 len) {
    int i;  
    for (i=0; i<len; ++i)
    { 
        TX1_write2buff(i);
        TX1_write2buff(str[i]);
    }
} 
 
void send_at_command(char *r)
{
    u8 i;  
    u8 len = strlen(r);
    for (i=0; i<len; ++i)
        TX1_write2buff(r[i]);
}


// LOCK
 void uart_send_test_lock()
 {
    strcpy(command, "+LOCK:(0-1)\r\nOK\r\n");
    send_at_command(command);
 } 
  
 void uart_send_open_success()
 {
    strcpy(command, "OK\r\n");
    send_at_command(command);
 } 


 void uart_send_open_fail()
 {
    strcpy(command, "+CCSHU:ERROR\r\n");
    send_at_command(command);
 } 


 void uart_send_lock_by_hand_success()
 {
    strcpy(command, "+LOCK:1\r\nOK\r\n");
    send_at_command(command);
 }

 void uart_send_lock_by_hand_fail()
 {
    strcpy(command, "+LOCK:0\r\nERROR\r\n");
    send_at_command(command);
 }

 void uart_send_status_now()
 {
    // todo
    if(status == on)
      strcpy(command, "+LOCK:0\r\nOK\r\n");
    else
      strcpy(command, "+LOCK:1\r\nOK\r\n");
    send_at_command(command);
 }


 // ALARM
 void uart_send_test_beep_success()
 {
    strcpy(command, "+ALARM:(0-1)\r\nOK\r\n");
    send_at_command(command);
    // uart_send_command(beep_callback_success);
 }

  void uart_send_beep_success()
 {
    strcpy(command, "OK\r\n");
    send_at_command(command);
    // uart_send_command(beep_callback_success);
 }

 void uart_send_beep_stop(){
    strcpy(command,"+ALARM:0\r\n");
    send_at_command(command);
 }

 // ALARMPARA
// +ALARMPARA:"KEY","VALUE"\r\nOK\r\n
  void uart_send_test_alarmpara()
 {
    strcpy(command, "+ALARMPARA:\"KEY\",\"VALUE\"\r\nOK\r\n");
    send_at_command(command);
    // uart_send_command(beep_callback_success);
 }

 void uart_send_alarmpara_success()
 {
    strcpy(command, "OK\r\n");
    send_at_command(command);
 }

 void uart_send_alarmpara_fail(){
    strcpy(command, "+CCSHU:ERROR\r\n");
    send_at_command(command);
 }


 // RESERVE
 void uart_send_test_schedule_callback()
 {
    strcpy(command, "+RESERVE:(0-1)\r\nOK\r\n");
    send_at_command(command);
 }

  void uart_send_schedule_callback()
 {
    strcpy(command, "OK\r\n");
    send_at_command(command);
 }

 void uart_send_status_schedule_callback()
 {
    strcpy(command, "+RESERVE:1\r\nOK\r\n");
    send_at_command(command);
 }


 // FINDME 
 void uart_send_test_trace_callback()
 {
    strcpy(command, "+FINDME:1\r\nOK\r\n");
    send_at_command(command);
 }

 void uart_send_trace_callback()
 {
    strcpy(command, "OK\r\n");
    send_at_command(command);
 }

 void uart_send_trace_fail_callback()
 {
    strcpy(command, "+CCSHU:ERROR\r\n");
    send_at_command(command);
 }

// +DIAGNOSE?
 void uart_send_diagnose_callback(){
    strcpy(command, "OK\r\n");
    send_at_command(command);
 }
 
 static void GetString1(u8 *received) {
    u8 len = COM1.RX_Cnt;  
    if (COM1.RX_Cnt)  
        memcpy(received, RX1_Buffer, COM1.RX_Cnt); 
     received[COM1.RX_Cnt] = '\0';
     memset(RX1_Buffer, 0, COM_RX1_Lenth); 
     USART_Finish_Rx(&COM1); 
 } 


  static void uart_dispatch_cmd(const char  *cmd) {
        if (memcmp(cmd, "+LOCK", 5) == 0) {
            if(cmd[5] == '=' && cmd[6] == '?'){
                // AT+LOCK=?
                // 返回：+LOCK:(0-1)\r\nOK\r\n
                uart_send_test_lock();
                reload_sleep_count_down();
            } else if(cmd[5] == '=' && cmd[6] == '0'){
                // AT+LOCK=0
                // 开锁
                // 返回OK\r\n
                cancle_lock_count_down();
                cnt_on_time_plus = 0;
                k_power_enable();
                reload_open_count_down();
                reload_open_beep_count_down();
                lock_on_detect_signal = 0;
                led_mode_set = open;
                led_mode_changed();
            } else if(cmd[5] == '=' && cmd[6] == '1'){
                // AT+LOCK=1
                // 关锁
                // 返回OK\r\n
                uart_send_open_success();
                reload_sleep_count_down();
            } else if(cmd[5] == '?'){
                // AT+LOCK=？
                // 查询状态
                // todo：return the status after get the status 16 adc
                k_power_enable();
                init_lock_para();
                // uart_send_status_now();
            } else {
                reload_sleep_count_down();
            } 
            
        } else if(memcmp(cmd, "+ALARM=", 7) == 0){
            if(cmd[7] == '?'){
                // AT+ALARM=?
                // 测试
                uart_send_test_beep_success();
            } else if(cmd[7] == '1'){
                // AT+ALARM=1
                // 报警开启
                // OK\r\n
                speak_alter_pca_init();
                speak_mode = alter_beep;
                reload_alter_beep_count_down();
                uart_send_beep_success();
            } else if(cmd[7] == '0'){
                // AT+ALARM=0
                // 取消
                uart_send_beep_success();
                cancle_alter_count_down();
                speak_beep_disable();
                speak_mode = mute;
                reload_sleep_count_down();
            } else {
                reload_sleep_count_down();
            } 
        } else if(memcmp(cmd, "+ALARMPARA=", 11) == 0){
            if(cmd[11] == '?'){
                // AT+ALARMPARA=?
                // 返回：+ALARMPARA:"KEY","VALUE"\r\nOK\r\n
                uart_send_test_alarmpara();
            } else {
                // AT+ALARMPARA="KEY","VALUE"
                if (cmd[11] == '\"' && cmd[16] == '\"'){
                    alter_time = ((cmd[12] - '0') * 1000 + (cmd[13] - '0') * 100 + (cmd[14] - '0') * 10 + (cmd[15] - '0')) / 20;
                    alter_flag = cmd[19] - '0';
                    uart_send_alarmpara_success(); 
                } else if(cmd[11] == '\"' && cmd[17] == '\"'){
                    alter_time = ((cmd[12] - '0') * 10000 + (cmd[13] - '0') * 1000 + (cmd[14] - '0') * 100 + (cmd[15] - '0') * 10 + (cmd[16] - '0')) / 20;
                    alter_flag = cmd[20] - '0';
                    uart_send_alarmpara_success();
                } else {
                    // 返回
                    uart_send_alarmpara_fail();
                    reload_sleep_count_down();
                }

            } 
        } else if (memcmp(cmd, "+FINDME=", 8) == 0) {
            if(cmd[8] == '?'){
                // AT+FINDME=?
                uart_send_test_trace_callback();
            } else if(cmd[8] == '1'){
                // AT+FINDME=?
                // 检测锁状态
                // 返回：+ALARM:(0-1)\r\nOK\r\n
                // reload_sleep_count_down();
                reload_cnt_for_beep();
                reload_trace_beep_count_down();
                led_mode_set = trace;
                led_mode_changed();
            } else {
                reload_sleep_count_down();
            } 
        } else if(memcmp(cmd, "+RESERVE=", 9) == 0){
            if(cmd[9] == '?'){
                // AT+LOCK=?
                // 返回：+ALARM:(0-1)\r\nOK\r\n
                uart_send_status_schedule_callback();
            } else if(cmd[9] == '1') {
                // AT+LOCK=<status>
                // 开锁或关锁
                // 返回OK\r\n
                reload_schedule_sleep_count_down();
                led_mode_set = schedule;
                led_mode_changed();
                uart_send_schedule_callback();
            } else if(cmd[9] == '0') {
                // AT+LOCK=<status>
                // 开锁或关锁
                // 返回OK\r\n
                led_display_stop();
                reload_sleep_count_down();
                uart_send_schedule_callback();
            } else {
                reload_sleep_count_down();
            } 
        } 
        // AT+DIAGNOSE􀒘
        else if(memcmp(cmd, "+DIAGNOSE?", 10) == 0){
            uart_send_diagnose_callback();
            reload_sleep_count_down();
        } 
        else {
            reload_sleep_count_down();
        } 
}


void uart_cache_response(char xdata *r) 
{
    char *token;
    memset(cached_response_line, 0, RESP_BUF_SIZE);
    strcpy(cached_response_line, r);
    token = strchr(cached_response_line, '+');
    if(token)
        uart_dispatch_cmd(token);
    else 
        reload_sleep_count_down();
}

static void PrintString1(u8 *puts) 
 { 
     for (; *puts != 0;  puts++)  TX1_write2buff(*puts);     //遇到停止符0结束 
 } 
 
 
 void uart_Init() { 
    memset(TX1_Buffer, 0, COM_TX1_Lenth); 
    memset(RX1_Buffer, 0, COM_RX1_Lenth); 
    USART_Init(&COM1); 
 
 
    S1_8bit(); 
    S1_BRT_UseTimer2();     // S1 BRT Use Timer2; 
    S1_Int_Enable();        // 允许中断 
    S1_RX_Enable();         // Rx 接收允许 
 
    S1_USE_P30P31(); 
    quasi_bidirectional(3, 0x01); 
    quasi_bidirectional(3, 0x02);
 
     S1_TXD_RXD_OPEN(); 
 
 
     USART_Share_Timer2(); 
 } 
 

 void uart_process_recieved() { 
     // char received[COM_RX1_Lenth];
     if(led_mode_set != schedule)
        cancle_sleep_count_down();
     uart_cache_response(RX1_Buffer);
     USART_Finish_Rx(&COM1);  
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
