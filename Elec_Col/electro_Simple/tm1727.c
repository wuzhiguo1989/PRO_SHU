
#include "tm1727.h"
#include "intrins.h"
#include "share.h"
#include "jl17467pqw.h"

#define display_delay() _nop_()

static u8 code clean_code[20]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
/********************Start函数*************************/
//开始信号
static void Tm1727_Start()                   
{
  SCL = 1;
  display_delay();
  SDA = 1;
  display_delay();
  SDA = 0;
  display_delay();
}


/********************Stop函数*************************/
//停止信号
static void Tm1727_Stop()          
{
  SCL = 1;
  display_delay();
  SDA = 0;
  display_delay();
  SDA = 1;
  display_delay();
}


/********************ACK函数*************************/
//应答信号
static void Tm1727_Ack()                  
{
  while(SDA);              //第8个时钟后等待芯片把总线拉低
  display_delay();
  SCL = 1;                 //主机给一个上升沿
  display_delay();
  SCL = 0;
  display_delay();
}


/***************发送8bit数据，从低位开始**************/
//写一字节数据程序
static void Tm1727_Write_Byte(unsigned char dat)      
{
  u8 i;
  for(i = 0;i < 8;i++)
  {
    if(dat & 0x01)     //检测SDA电平
      SDA = 1;
    else 
      SDA = 0;
    dat>>=1;
    display_delay();
    SCL = 0;           //SCL上升沿数据被写入
    display_delay();
    SCL = 1;
    display_delay();
    SCL = 0;
  }
}

// 写入20个RAM地址的数据
static void Tm1727_Write_Data(u8 *ram_code) 
{
  u8 i;
  for(i=0;i<20;i++)
  {
    Tm1727_Write_Byte(*ram_code);
    ram_code++;
    Tm1727_Ack();
  } 
}



/*****************发送显示控制命令*****************/
//发送显示控制命令
static void Tm1727_Send_Display_cmd(u8 dat)                   
{
  Tm1727_Start();           //开始信号
  Tm1727_Write_Byte(dat);   //写入一个8bit的数据，低位在前高位在后
  Tm1727_Ack();             //应答信号
  Tm1727_Stop();            //停止信号
}


static void Tm1727_Send_Start_Address(u8 add)
{
  Tm1727_Start();
  Tm1727_Write_Byte(add);    //起始地址
  Tm1727_Ack();
}

/***************设置位**************/
void Tm1727_Local_Set_Bit(const u8 line, const u8 column, const u8 value) {
  if(!value)
    display_code[line] &= ~(1 << column);
  else
    display_code[line] |= (1 << column);
}

void display_clean()
{
  Tm1727_Send_Start_Address(RAM_START_ADDRESS);
  Tm1727_Write_Data(clean_code);
  Tm1727_Stop();
  Tm1727_Send_Display_cmd(0x27);    //发送显示控制命令：正常模式，LINE模式，1/3偏压，5V
  jt17467pqw_set_separator(1);
}


/***************显示函数**************/
void display(u8 add, u8 *ram_code)
{
  Tm1727_Send_Start_Address(add);
  Tm1727_Write_Data(ram_code);
  Tm1727_Stop();
  Tm1727_Send_Display_cmd(0x27);    //发送显示控制命令：正常模式，LINE模式，1/3偏压，5V
}   
  





