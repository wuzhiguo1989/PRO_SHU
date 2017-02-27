 #include "lm567.h" 
 #include "string.h" 
 #include "usart.h" 
 #include "task.h" 
 #include "config.h" 
 #include "stdio.h" 
 #include "sync.h" 
 #include "utils.h" 
 #include "key.h" 
 #include "power.h"  
 #include "display.h" 
 
 
 
 
 #define ble_send_command(x)   PrintString1(x) 
 #define ble_recieve_data(x)   GetString1(x) 
 static volatile u8 xdata TX1_Buffer[COM_TX1_Lenth];    //发送缓冲 
 static volatile u8 xdata RX1_Buffer[COM_RX1_Lenth];    //接收缓冲 
 
 
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
 
 
 void BLE_Init() { 
 
 
     memset(TX1_Buffer, 0, COM_TX1_Lenth); 
     memset(RX1_Buffer, 0, COM_RX1_Lenth); 
     USART_Init(&COM1); 
 
 
     S1_8bit(); 
     S1_BRT_UseTimer2();     // S1 BRT Use Timer2; 
 
 
     S1_Int_Enable();        // 允许中断 

 
     S1_RX_Enable();         // Rx 接收允许 
    S1_USE_P36P37(); 
    quasi_bidirectional(3, 0x80); 
    quasi_bidirectional(3, 0x40); 
  //  S1_USE_P30P31(); 
		// quasi_bidirectional(3, 0x01); 
		// quasi_bidirectional(3, 0x02);
 
     S1_TXD_RXD_OPEN(); 
 
 
     USART_Share_Timer2(); 
 } 
 
 
 #define GP228_CMD_SIZE   4 
 
 
 static void uart_dispatch_gp228_cmd(const char xdata *cmd) { 
     // A  mode 
     // B  power 
     // C  color 
     // D  alarm 
     // E  lumins 
     // F  speed
     if (cmd[0] == 'A') { 
        ctrl_mode_changed((cmd[1] - '0') * 10 + (cmd[2] - '0')); 
     } else if (cmd[0] == 'B') { 
        power_level_set = cmd[1] - '0';
        power_select_level(); 
     } else if (cmd[0] == 'C'){ 
        led_syc_color(cmd); 
     } else if (cmd[0] == 'D'){ 
        countdown_off(cmd[1] - '0'); 
     } else if (cmd[0] == 'E'){
        //todo 
        ctrl_lumins_changed((cmd[1] - '0') * 10 + (cmd[2] - '0'));
     } else if (cmd[0] == 'F') { 
        ctrl_speed_level_changed((cmd[1] - '0') * 10 + (cmd[2] - '0')); 
     }  
  } 
 
 
 void uart_process_recieved() { 
     char received[COM_RX1_Lenth]; 
     ble_recieve_data(received); 
     uart_dispatch_gp228_cmd(received);  
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
