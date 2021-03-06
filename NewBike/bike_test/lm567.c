 #include "lm567.h" 
 #include "string.h" 
 #include "usart.h" 
 #include "task.h" 
 #include "stdio.h" 
 #include "sync.h" 
 #include "utils.h" 
 #include "key.h" 
 #include "power.h" 
 #include "beep.h" 
 

#define SLIP_START 0x7E
#define SLIP_END   0x7F
#define SLIP_REPL  0x7D
#define SLIP_ESC   0x20
#define CMD_CNT    8
#define CNT_MAX_LOCK_RESEND 600


volatile u16 cnt_lock = 1;
volatile u8 cnt_lock_by_use = 0;
volatile u8 cnt_restart = 1;
volatile u8 lock_from_control = 0;

static volatile u8 xdata TX1_Buffer[COM_TX1_Lenth];    //发送缓冲
static volatile u8 xdata RX1_Buffer[COM_RX1_Lenth];    //接收缓冲

static code u8 open_callback_success[] = {0x7E, 0x81, 0x00, 0x00, 0x81, 0x7F};
static code u8 open_callback_fail[] = {0x7E, 0x81, 0x00, 0x01, 0x82, 0x7F};
static code u8 lock_callback_by_hand_success[] = {0x7E, 0x84, 0x00, 0x00, 0x84, 0x7F};
static code u8 lock_callback_by_hand_fail[] = {0x7E, 0x84, 0x00, 0x01, 0x85, 0x7F};
static code u8 beep_callback_success[] = {0x7E, 0x83, 0x00, 0x00, 0x83, 0x7F};
static code u8 beep_callback_fail[] = {0x7E, 0x83, 0x00, 0x00, 0x83, 0x7F};
static code u8 version1_callback[] = {0x7E, 0x88, 0x01, 0x00, 0x89, 0x7F};
static code u8 schedule_success[] =  {0x7E, 0x86, 0x00, 0x00, 0x86, 0x7F};
static code u8 trace_success[] = {0x7E, 0x87, 0x00, 0x00, 0x87, 0x7F};



 
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
 
void uart_send_command(u8 *r)
{
    u8 i;
    for(i = 0; i < CMD_CNT; i++)
        TX1_write2buff(r[i]);
}

  
 void uart_send_open_success()
 {
    u8 lock_status;
    TX1_write2buff(0x7E);
    TX1_write2buff(0x81);
    if(INT1 == 0 && status == off){
        lock_status = 0x00;
    } else if(INT1 == 1 && status == on){
        lock_status = 0x11;
    } else if(INT1 == 0 && status == on){
        lock_status = 0x10;
    } else if(INT1 == 1 && status == off){
        lock_status = 0x01;
    }
    TX1_write2buff(lock_status);
    TX1_write2buff(0x00);
    TX1_write2buff(0x81 + lock_status);
    TX1_write2buff(0x7F);
   // uart_send_command(open_callback_success);
 } 


 void uart_send_lock_success()
 {
    u8 lock_status;
    TX1_write2buff(0x7E);
    TX1_write2buff(0x82);
    if(INT1 == 0 && status == off){
        lock_status = 0x00;
    } else if(INT1 == 1 && status == on){
        lock_status = 0x11;
    } else if(INT1 == 0 && status == on){
        lock_status = 0x10;
    } else if(INT1 == 1 && status == off){
        lock_status = 0x01;
    }
    TX1_write2buff(lock_status);
    TX1_write2buff(0x00);
    TX1_write2buff(0x82 + lock_status);
    TX1_write2buff(0x7F);
}
 

 void uart_send_open_fail()
 {
    uart_send_command(open_callback_fail);
 } 


  void uart_send_beep_success(u8 cmd)
 {
    TX1_write2buff(0x7E);
    TX1_write2buff(0x83);
    TX1_write2buff(cmd);
    TX1_write2buff(0x00);
    TX1_write2buff(0x83 + cmd);
    TX1_write2buff(0x7F);
    // uart_send_command(beep_callback_success);
 }

 //  void uart_send_lock_by_hand_success()
 // {
 //    uart_send_command(lock_callback_by_hand_success);
 // }

 //   void uart_send_lock_by_hand_fail()
 // {
 //    uart_send_command(lock_callback_by_hand_fail);
 // }

 void uart_send_lock_by_hand_success()
 {
    u8 lock_status;
    u8 cmd;
    if(INT1 == 0 && status == off){
        lock_status = 0x00;
    } else if(INT1 == 1 && status == on){
        lock_status = 0x11;
    } else if(INT1 == 0 && status == on){
        lock_status = 0x10;
    } else if(INT1 == 1 && status == off){
        lock_status = 0x01;
    }
    cmd = 0x84 + cnt_lock_by_use + lock_status;
    TX1_write2buff(0x7E);
    TX1_write2buff(0x84);
    TX1_write2buff(lock_status);
    if(cnt_lock_by_use == 0x7E){
        TX1_write2buff(0x7D);
        TX1_write2buff(0x5E);
    } else if(cnt_lock_by_use == 0x7F){
        TX1_write2buff(0x7D);
        TX1_write2buff(0x5F);
    } else if(cnt_lock_by_use == 0x7D){
        TX1_write2buff(0x7D);
        TX1_write2buff(0x5D);
    } else {
       TX1_write2buff(cnt_lock_by_use); 
    }

    if(cmd == 0x7E){
        TX1_write2buff(0x7D);
        TX1_write2buff(0x5E);
    } else if(cmd == 0x7F){
        TX1_write2buff(0x7D);
        TX1_write2buff(0x5F);
    } else if(cmd == 0x7D){
        TX1_write2buff(0x7D);
        TX1_write2buff(0x5D);
    } else {
       TX1_write2buff(cmd); 
    }
    TX1_write2buff(0x7F);
    if(cnt_lock == CNT_MAX_LOCK_RESEND){
        resend_lock_command_signal = 0;
    }
    //     u8 cmd = 0x84 + cnt_lock;
    // TX1_write2buff(0x7E);
    // TX1_write2buff(0x84);
    // TX1_write2buff(0x00);
    // if(cnt_lock == 0x7E){
    //     TX1_write2buff(0x7D);
    //     TX1_write2buff(0x5E);
    // } else if(cnt_lock == 0x7F){
    //     TX1_write2buff(0x7D);
    //     TX1_write2buff(0x5F);
    // } else if(cnt_lock == 0x7D){
    //     TX1_write2buff(0x7D);
    //     TX1_write2buff(0x5D);
    // } else {
    //    TX1_write2buff(cnt_lock); 
    // }

    // if(cmd == 0x7E){
    //     TX1_write2buff(0x7D);
    //     TX1_write2buff(0x5E);
    // } else if(cmd == 0x7F){
    //     TX1_write2buff(0x7D);
    //     TX1_write2buff(0x5F);
    // } else if(cmd == 0x7D){
    //     TX1_write2buff(0x7D);
    //     TX1_write2buff(0x5D);
    // } else {
    //    TX1_write2buff(cmd); 
    // }
    // TX1_write2buff(0x7F);
}

 void uart_send_lock_by_hand_fail()
 {
    u8 cmd = 0x85 + cnt_lock;
    TX1_write2buff(0x7E);
    TX1_write2buff(0x84);
    TX1_write2buff(0x01);
    if(cnt_lock == 0x7E){
        TX1_write2buff(0x7D);
        TX1_write2buff(0x5E);
    } else if(cnt_lock == 0x7F){
        TX1_write2buff(0x7D);
        TX1_write2buff(0x5F);
    } else if(cnt_lock == 0x7D){
        TX1_write2buff(0x7D);
        TX1_write2buff(0x5D);
    } else {
       TX1_write2buff(cnt_lock); 
    }

    if(cmd == 0x7E){
        TX1_write2buff(0x7D);
        TX1_write2buff(0x5E);
    } else if(cmd == 0x7F){
        TX1_write2buff(0x7D);
        TX1_write2buff(0x5F);
    } else if(cmd == 0x7D){
        TX1_write2buff(0x7D);
        TX1_write2buff(0x5D);
    } else {
       TX1_write2buff(cmd); 
    }
    TX1_write2buff(0x7F);
 }

 void uart_send_status_now(u8 lock_status, u8 sys_status)
 {
    TX1_write2buff(0x7E);
    TX1_write2buff(0x85);
    TX1_write2buff(lock_status);
    TX1_write2buff(sys_status);
    TX1_write2buff(0x85 + lock_status + sys_status);
    TX1_write2buff(0x7F);
 }

 void uart_send_schedule_callback()
 {
    // uart_send_command(schedule_success);
    TX1_write2buff(0x7E);
    TX1_write2buff(0x86);
    TX1_write2buff(0x00);
    TX1_write2buff(0x00);
    TX1_write2buff(0x86);
    TX1_write2buff(0x7F);
 }

  void uart_send_trace_callback()
 {
    // TX1_write2buff(0x7E);
    // TX1_write2buff(0x86);
    // TX1_write2buff(0x00);
    // TX1_write2buff(0x00);
    // TX1_write2buff(0x86);
    // TX1_write2buff(0x7F);
 }


 void uart_send_status_restart()
 {
    u8 cmd;
    u8 lock_status;
    if(INT1 == 0 && status == off){
        lock_status = 0x00;
    } else if(INT1 == 1 && status == on){
        lock_status = 0x11;
    } else if(INT1 == 0 && status == on){
        lock_status = 0x10;
    } else if(INT1 == 1 && status == off){
        lock_status = 0x01;
    }
    cmd = 0x87 + lock_status + cnt_restart;
    TX1_write2buff(0x7E);
    TX1_write2buff(0x87);
    TX1_write2buff(lock_status);
    if(cnt_restart == 0x7E){
        TX1_write2buff(0x7D);
        TX1_write2buff(0x5E);
    } else if(cnt_restart == 0x7F){
        TX1_write2buff(0x7D);
        TX1_write2buff(0x5F);
    } else if(cnt_restart == 0x7D){
        TX1_write2buff(0x7D);
        TX1_write2buff(0x5D);
    } else {
       TX1_write2buff(cnt_restart); 
    }

    if(cmd == 0x7E){
        TX1_write2buff(0x7D);
        TX1_write2buff(0x5E);
    } else if(cmd == 0x7F){
        TX1_write2buff(0x7D);
        TX1_write2buff(0x5F);
    } else if(cmd == 0x7D){
        TX1_write2buff(0x7D);
        TX1_write2buff(0x5D);
    } else {
       TX1_write2buff(cmd); 
    }
    TX1_write2buff(0x7F);
    if(cnt_restart == 0xFF){
        cancel_resend_restart_count_down();
        EX1 = ENABLE; 
        reload_sleep_count_down();
    }
}

 void uart_send_version1_callback(){
    uart_send_command(version1_callback);
 }
 
 static void GetString1(u8 *received) {
    u8 len = COM1.RX_Cnt;  
    if (COM1.RX_Cnt)  
        memcpy(received, RX1_Buffer, COM1.RX_Cnt); 
     received[COM1.RX_Cnt] = '\0';
     memset(RX1_Buffer, 0, COM_RX1_Lenth); 
     USART_Finish_Rx(&COM1); 
 } 


  static void uart_dispatch_cmd(const char  *r) {
        switch(r[0])
        {
            // 开锁命令
            case 0x01:
                cancle_lock_count_down();
                cancel_lock_delay();
                cnt_on_time_plus = 0;
                k_power_enable();
                power_open_work();
                reload_open_count_down();
                reload_open_beep_count_down();
                reload_open_delay();
                // lock_on_detect_signal = 0;
                led_mode_set = open;
                led_mode_changed();
                break;
            case 0x02:
                cancle_sleep_count_down();
                cancle_open_count_down();
                cancel_open_delay();
                reload_lock_count_down();
                k_power_enable();
                reload_lock_delay();
                lock_from_control = 1;
                cnt_off_time_plus = 0; 
                break;
            // 报警命令
            case 0x03:
                if(r[1] == 0x02)
                {
                    if(led_mode_set != schedule)
                        reload_alter_sleep_count_down();
                    speak_alter_pca_init();
                    speak_beep_enable();
                    speak_mode = alter_beep;
                    reload_alter_beep_count_down();
                }
                cpu_wake_up_enable();
                uart_send_beep_success(r[1]);
                cpu_wake_up_disable();
                break;

            // 接收到上锁命令
            case 0x04:
                // 无需反馈
                resend_lock_command_signal = 0;
                cnt_lock = 1;
                reload_sleep_count_down();
                break;

            // 查询状态
            case 0x05:
                k_power_enable();
                init_lock_para();
                break;

            // 预约/取消预约
            case 0x06:
                // 不需要 直接反馈
                if(r[1] == 0x01){
                    reload_schedule_sleep_count_down();
                    led_mode_set = schedule;
                    led_mode_changed();
                } else if(r[1] == 0x00){
                    cancle_sleep_count_down();
                    // 准备睡眠
                    led_display_stop();
                    // led_mode_changed();
                    reload_sleep_count_down();
                }

                INT_PROC |= SCHE_DUTY;
                break;
            // 重启响应命令     
            case 0x07:
                cnt_restart = 1;
                cancel_resend_restart_count_down();
                reload_sleep_count_down();
                break;
            // 查询版本号
            case 0x08:
                uart_send_version1_callback();
                reload_sleep_count_down();
                break;
            default:
                if(led_mode_set != schedule) 
                    reload_sleep_count_down();
                break;           
        }
 
  }


void uart_cache_response(char xdata *r, char xdata *received) 
{
    u8 index = 1, cmd_index = 0;
    u8 cmd[COM_RX1_Lenth - 2];
    u8 len = COM1.RX_Cnt;
    cancle_sleep_count_down();
    if(len < 6 || len > 9)
        USART_Finish_Rx(&COM1);
    else 
    {
        memset(received, 0, COM_RX1_Lenth);
        memcpy(received, r, sizeof(char) * len);
        USART_Finish_Rx(&COM1);
        if(received[0] == SLIP_START && received[len - 1] == SLIP_END)
        {
        // todo 处理 check
            while(index <= len - 2)
            {
                if(received[index] != SLIP_REPL){
                    cmd[cmd_index] = received[index];
                    cmd_index ++;
                    index ++;
                } 
                else
                {
                    if(received[index + 1] == 0x5E){
                        cmd[cmd_index] = 0x7E;
                    } else if(received[index + 1] == 0x5F){
                     cmd[cmd_index] = 0x7F;
                    } else if(received[index + 1] == 0x5D){
                     cmd[cmd_index] = 0x7D;
                    } else {
                     if(led_mode_set != schedule)
                        reload_sleep_count_down();
                     goto  error_handle;
                    } 
                    cmd_index ++;
                    index = index + 2;
                }
                if(cmd[3] == cmd[0] +  cmd[1] + cmd[2]){
                    uart_dispatch_cmd(cmd);
                } else {
                    if(led_mode_set != schedule)
                    reload_sleep_count_down();
                }
        
            }
        }   
        else
        {
            if(led_mode_set != schedule)
                reload_sleep_count_down();
            goto  error_handle;
        }
        error_handle: _nop_();
    }
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
     char received[COM_RX1_Lenth];
     if(led_mode_set != schedule)
        cancle_sleep_count_down();
     uart_cache_response(RX1_Buffer, received);  
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
