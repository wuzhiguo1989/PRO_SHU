// nRF24L01.h
#ifndef _NRF_24L01_
#define _NRF_24L01_

//==NRF24L01===============================================================================
/*-----------------------------SPI相关寄存器表----------------------------------------------*/
// SPCTL 控制寄存器   BIT7   BIT6  BIT5   BIT4   BIT3  BIT2 BIT1 BIT0
//                   SSIG   SPEN  DORD   MSTR   CPOL  CPHA SPR1  SPR0
#define SPI_STAT      0X40   //使能SPI
#define SPI_FLASH     0x9C   //空闲时钟高电平  第二边沿采样  第一位为最高位    速率 CPU_CLK/4   未开启SPI
#define SPI_NRF24L01  0x90   //空闲时钟低电平  第一边沿采样  第一位为最高位    速率 CPU_CLK/4   未开启SPI

//SPSTAT  状态寄存器    BIT7  SPIF   传输完成标志    BIT6  写冲突标志    对应位写1清零

// SPDAT  数据寄存器
// IE      BIT7  EA       总中断开关
// IE2    中断允许寄存器    BIT1  ESPI   允许SPI中断位
// IP2    中断优先级控制寄存器  BIT1  PSPI    0最低优先级中断     1最高优先级
// P_SW1/AUXR1   外设切换控制寄存器1   BIT3 BIT2  SPI_S1 SPI_S0   
// 00  P12 P13 P14 P15   01 P24 P23 P22 P21    10 P54 P40 P41 P43   11无效   引脚分别对应  SS  MOSI  MISO   SCLK

extern void SPI_Init(void);  //空闲时钟高电平  第二边沿采样  第一位为最高位    速率 CPU_CLK/4  
extern void SPI_SWITCH(unsigned char num); //SPI引脚切换 0 P12 P13 P14 P15, 1 P24 P23 P22 P21, 2 P54 P40 P41 P43  其它无效 引脚分别对应  SS MOSI MISO SCLK
extern void SPI_SPEED(unsigned char num);  //设置SPI 速度  0~3  0为最大速度
extern unsigned char SPI_WRITE_READ(unsigned dat);  //SPI 读写字节

//SPI接口------------------  空闲时钟为低电平  第一个边沿信号采集数据  最大速度10M-----------------------
/******************所用硬件SPI*************************************
#define MOSI    P13   
#define MISO    P14
#define SCLK    P15
*******************************************************************/

#define NRF24L01_IRQ     P36  // 中断信号引脚  低电平为中断信号
#define NRF24L01_CSN     P12  // SPI 使能信号  片选信号
#define NRF24L01_CE      P37  // 无线模块使能信号   高电平接收模式  低电休眠模式  上升沿（大于10us）发送数据    只有在CE为低电平时才能写寄存器

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//NRF24L01寄存器操作命令
#define READ_REG_NRF    0x00  //读配置寄存器,低5位为寄存器地址
#define WRITE_REG_NRF   0x20  //写配置寄存器,低5位为寄存器地址
#define RD_RX_PLOAD     0x61  //读RX有效数据,1~32字节
#define WR_TX_PLOAD     0xA0  //写TX有效数据,1~32字节
#define FLUSH_TX        0xE1  //清除TX FIFO寄存器.发射模式下用
#define FLUSH_RX        0xE2  //清除RX FIFO寄存器.接收模式下用
#define REUSE_TX_PL     0xE3  //重新使用上一包数据,CE为高,数据包被不断发送.
#define NOP             0xFF  //空操作,可以用来读状态寄存器	 
//SPI(NRF24L01)寄存器地址
#define CONFIG          0x00  //配置寄存器地址;bit0:1接收模式,0发射模式;bit1:电选择;bit2:CRC模式;bit3:CRC使能;
                              //bit4:中断MAX_RT(达到最大重发次数中断)使能;bit5:中断TX_DS使能;bit6:中断RX_DR使能
#define EN_AA           0x01  //使能自动应答功能  bit0~5,对应通道0~5
#define EN_RXADDR       0x02  //接收地址允许,bit0~5,对应通道0~5
#define SETUP_AW        0x03  //设置地址宽度(所有数据通道):bit1,0:00,3字节;01,4字节;02,5字节;
#define SETUP_RETR      0x04  //建立自动重发;bit3:0,自动重发计数器;bit7:4,自动重发延时 250*x+86us
#define RF_CH           0x05  //RF通道,bit6:0,工作通道频率;
#define RF_SETUP        0x06  //RF寄存器;bit3:传输速率(0:1Mbps,1:2Mbps);bit2:1,发射功率;bit0:低噪声放大器增益
#define STATUS          0x07  //状态寄存器;bit0:TX FIFO满标志;bit3:1,接收数据通道号(最大:6);bit4,达到最多次重发
                              //bit5:数据发送完成中断;bit6:接收数据中断;
//状态判别-------------															
#define MAX_TX  		  0x10  //达到最大发送次数中断
#define TX_OK   		  0x20  //TX发送完成中断
#define RX_OK   		  0x40  //接收到数据中断

#define OBSERVE_TX      0x08  //发送检测寄存器,bit7:4,数据包丢失计数器;bit3:0,重发计数器
#define CD              0x09  //载波检测寄存器,bit0,载波检测;
#define RX_ADDR_P0      0x0A  //数据通道0接收地址,最大长度5个字节,低字节在前
#define RX_ADDR_P1      0x0B  //数据通道1接收地址,最大长度5个字节,低字节在前
#define RX_ADDR_P2      0x0C  //数据通道2接收地址,最低字节可设置,高字节,必须同RX_ADDR_P1[39:8]相等;
#define RX_ADDR_P3      0x0D  //数据通道3接收地址,最低字节可设置,高字节,必须同RX_ADDR_P1[39:8]相等;
#define RX_ADDR_P4      0x0E  //数据通道4接收地址,最低字节可设置,高字节,必须同RX_ADDR_P1[39:8]相等;
#define RX_ADDR_P5      0x0F  //数据通道5接收地址,最低字节可设置,高字节,必须同RX_ADDR_P1[39:8]相等;
#define TX_ADDR         0x10  //发送地址(低字节在前),ShockBurstTM模式下,RX_ADDR_P0与此地址相等
//通道接收有效数据宽度----------------------------
#define RX_PW_P0        0x11  //接收数据通道0有效数据宽度(1~32字节),设置为0则非法
#define RX_PW_P1        0x12  //接收数据通道1有效数据宽度(1~32字节),设置为0则非法
#define RX_PW_P2        0x13  //接收数据通道2有效数据宽度(1~32字节),设置为0则非法
#define RX_PW_P3        0x14  //接收数据通道3有效数据宽度(1~32字节),设置为0则非法
#define RX_PW_P4        0x15  //接收数据通道4有效数据宽度(1~32字节),设置为0则非法
#define RX_PW_P5        0x16  //接收数据通道5有效数据宽度(1~32字节),设置为0则非法
#define NRF_FIFO_STATUS 0x17  //FIFO状态寄存器;bit0,RX FIFO寄存器空标志;bit1,RX FIFO满标志;bit2,3,保留
                              //bit4,TX FIFO空标志;bit5,TX FIFO满标志;bit6,1,循环发送上一数据包.0,不循环;
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//24L01发送接收数据宽度定义
#define TX_ADR_WIDTH    5   	//5字节的地址宽度
#define RX_ADR_WIDTH    5   	//5字节的地址宽度
#define TX_PLOAD_WIDTH  2  	//32字节的用户数据宽度
#define RX_PLOAD_WIDTH  1  	//32字节的用户数据宽度

void  NRF24L01_Init(void);
unsigned char      NRF24L01_Write_Reg(unsigned char   reg,unsigned char   value);          //指定寄存器写数据
unsigned char      NRF24L01_Read_Reg(unsigned char   reg);                    //读寄存器
unsigned char      NRF24L01_Read_Buf(unsigned char   reg,unsigned char   *pBuf,unsigned char   len);    //指定寄存器读指定长度
unsigned char      NRF24L01_Write_Buf(unsigned char   reg, unsigned char   *pBuf, unsigned char   len); //指定寄存器写指定长度
//发送----------------------------------------------------------------------
void  NRF24L01_TX_Mode(void);                       //配置为发送模式
unsigned char      NRF24L01_TxPacket(unsigned char   *txbuf);                 //发送一次数据  32字节
unsigned char      NRF24L01_TxPacket_NUM(unsigned char   *txbuf,unsigned char   num);         //指定接收地址发送0~5   大于5为默认发送地址发送
//接收--------------------------------------------------------------------
void  NRF24L01_RX_Mode(unsigned char  num);                       //设为接收模式                      //设为接收模式
// unsigned char      NRF24L01_RxPacket(unsigned char   *rxbuf,unsigned char   *num);         //接收一次数据 返回状态  获取通道
unsigned char      NRF24L01_RxPacket(unsigned char   *rxbuf);         //接收一次数据 返回状态  获取通道
#endif