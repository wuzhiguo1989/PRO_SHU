
#include "tm1727.h"
#include "intrins.h"
#include "share.h"
#include "jl17467pqw.h"

#define display_delay() _nop_()

static u8 code clean_code[20]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
/********************Start����*************************/
//��ʼ�ź�
static void Tm1727_Start()                   
{
  SCL = 1;
  display_delay();
  SDA = 1;
  display_delay();
  SDA = 0;
  display_delay();
}


/********************Stop����*************************/
//ֹͣ�ź�
static void Tm1727_Stop()          
{
  SCL = 1;
  display_delay();
  SDA = 0;
  display_delay();
  SDA = 1;
  display_delay();
}


/********************ACK����*************************/
//Ӧ���ź�
static void Tm1727_Ack()                  
{
  while(SDA);              //��8��ʱ�Ӻ�ȴ�оƬ����������
  display_delay();
  SCL = 1;                 //������һ��������
  display_delay();
  SCL = 0;
  display_delay();
}


/***************����8bit���ݣ��ӵ�λ��ʼ**************/
//дһ�ֽ����ݳ���
static void Tm1727_Write_Byte(unsigned char dat)      
{
  u8 i;
  for(i = 0;i < 8;i++)
  {
    if(dat & 0x01)     //���SDA��ƽ
      SDA = 1;
    else 
      SDA = 0;
    dat>>=1;
    display_delay();
    SCL = 0;           //SCL���������ݱ�д��
    display_delay();
    SCL = 1;
    display_delay();
    SCL = 0;
  }
}

// д��20��RAM��ַ������
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



/*****************������ʾ��������*****************/
//������ʾ��������
static void Tm1727_Send_Display_cmd(u8 dat)                   
{
  Tm1727_Start();           //��ʼ�ź�
  Tm1727_Write_Byte(dat);   //д��һ��8bit�����ݣ���λ��ǰ��λ�ں�
  Tm1727_Ack();             //Ӧ���ź�
  Tm1727_Stop();            //ֹͣ�ź�
}


static void Tm1727_Send_Start_Address(u8 add)
{
  Tm1727_Start();
  Tm1727_Write_Byte(add);    //��ʼ��ַ
  Tm1727_Ack();
}

/***************����λ**************/
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
  Tm1727_Send_Display_cmd(0x27);    //������ʾ�����������ģʽ��LINEģʽ��1/3ƫѹ��5V
  jt17467pqw_set_separator(1);
}


/***************��ʾ����**************/
void display(u8 add, u8 *ram_code)
{
  Tm1727_Send_Start_Address(add);
  Tm1727_Write_Data(ram_code);
  Tm1727_Stop();
  Tm1727_Send_Display_cmd(0x27);    //������ʾ�����������ģʽ��LINEģʽ��1/3ƫѹ��5V
}   
  





