//
// nRF24L01.c

#include "nRF24L01.h"
#include "stc15w408as.h"
//=======================硬件SPI初始化=================================================================================================
void SPI_Init(void) //空闲时钟高电平  第二边沿采样  第一位为最高位    速率 CPU_CLK/4  
{
   SPSTAT=0XC0;  //清零传输完成标注及写冲突标志
	 SPCTL=SPI_NRF24L01;   //NRF24L01   空闲时钟低电平  第一边沿采样  第一位为最高位    速率 CPU_CLK/4   未开启SPI
   SPCTL|=SPI_STAT;    //开启SPI
}

unsigned char SPI_WRITE_READ(unsigned dat)  //SPI 读写字节
{
   SPDAT =dat;               //写入寄存器
   while(!(SPSTAT & 0X80));  //等待传输完成
	 SPSTAT=0XC0;       //清零标志
	 return SPDAT;
}

void SPI_SWITCH(unsigned char num) //SPI引脚切换 0 P12 P13 P14 P15, 1 P24 P23 P22 P21, 2 P54 P40 P41 P43  其它无效 引脚分别对应  SS MOSI MISO SCLK
{
   	num=num&0x03;        //约束范围
	if(num==3)  return ; //无效操作
	P_SW1&=0xF3;         //清零之前操作
   	P_SW1|= num<<2;
}

void SPI_SPEED(unsigned char num)  //设置SPI 速度  0~3  0为最大速度
{
   	num=num&0x03;        //约束范围
	SPCTL&=~0x03;        //清零之前操作
   	SPCTL|= num; 
	SPCTL|=SPI_STAT;    //开启SPI
}
//===================================================================================================


//发送地址---------------------------------------------------------------------
code unsigned char  TX_ADDRESS[TX_ADR_WIDTH]={0x00,0x43,0x10,0x10,0x01}; //发送地址 一次只能向一个地址发送 5字节地址独立  发送时必须与RX_ADDR_P0地址相同

//接收通道地址-------------------------------------------------------------------------
code unsigned char  RX_ADDRESS[6][5]={                //6个接收通道     每个通道5个字节  第0和第1通道地址完全独立 第2~5通道低8位独立 高位与通道1相同！
													{0x00,0x43,0x10,0x10,0x01},   //5字节地址完全独立

													{0x01,0x43,0x10,0x10,0x01},   //接收通道1地址  5字节地址完全独立
													{0x02,0x43,0x10,0x10,0x01},   //接收通道2地址  低8位地址独立  高位与通道1相同
													{0x03,0x43,0x10,0x10,0x01},   //接收通道3地址  低8位地址独立  高位与通道1相同
													{0x04,0x43,0x10,0x10,0x01},   //接收通道4地址  低8位地址独立  高位与通道1相同
													{0x05,0x43,0x10,0x10,0x01},   //接收通道5地址  低8位地址独立  高位与通道1相同
                        };


//SPI写寄存器
//reg:指定寄存器地址
//value:写入的值
unsigned char  NRF24L01_Write_Reg(unsigned char  reg,unsigned char  value)
{
	  unsigned char  status;	
   	NRF24L01_CSN=0;                 //使能SPI传输
  	status =SPI_WRITE_READ(reg);	//发送寄存器号 
  	SPI_WRITE_READ(value);      	//写入寄存器的值
  	NRF24L01_CSN=1;                 //禁止SPI传输	   
  	return(status);       			//返回状态值
}

//读取SPI寄存器值
//reg:要读的寄存器
unsigned char  NRF24L01_Read_Reg(unsigned char  reg)
{
	unsigned char  reg_val;	    
 	NRF24L01_CSN = 0;          //使能SPI传输		
  SPI_WRITE_READ(reg);   //发送寄存器号
  reg_val=SPI_WRITE_READ(0XFF);//读取寄存器内容
  NRF24L01_CSN = 1;          //禁止SPI传输		    
  return(reg_val);           //返回状态值
}	

//在指定位置读出指定长度的数据
//reg:寄存器(位置)
//*pBuf:数据指针
//len:数据长度
//返回值,此次读到的状态寄存器值 
unsigned char  NRF24L01_Read_Buf(unsigned char  reg,unsigned char  *pBuf,unsigned char  len)
{
	  unsigned char  status,u8_ctr;	       
  	NRF24L01_CSN = 0;           //使能SPI传输
  	status=SPI_WRITE_READ(reg);//发送寄存器值(位置),并读取状态值   	   
 	for(u8_ctr=0;u8_ctr<len;u8_ctr++)pBuf[u8_ctr]=SPI_WRITE_READ(0XFF);//读出数据
  	NRF24L01_CSN=1;       //关闭SPI传输
  	return status;        //返回读到的状态值
}

//在指定位置写指定长度的数据
//reg:寄存器(位置)
//*pBuf:数据指针
//len:数据长度
//返回值,此次读到的状态寄存器值
unsigned char  NRF24L01_Write_Buf(unsigned char  reg, unsigned char  *pBuf, unsigned char  len)
{
	unsigned char  status,u8_ctr;	    
 	NRF24L01_CSN = 0;          //使能SPI传输
  status = SPI_WRITE_READ(reg);//发送寄存器值(位置),并读取状态值
  for(u8_ctr=0; u8_ctr<len; u8_ctr++)SPI_WRITE_READ(*pBuf++); //写入数据	 
  NRF24L01_CSN = 1;       //关闭SPI传输
  return status;          //返回读到的状态值
}	

void  NRF24L01_Init(void)
{
  SPI_Init();      //初始化SPI
	SPI_SPEED(0);    //约束速度   NRF24L01最大速度10M
	SPI_SWITCH(0);
  NRF24L01_CSN=1;  //关闭片选
	NRF24L01_IRQ=1;  //中断IO 输出高电平  类似上拉效果
	NRF24L01_CE=0;   //进入待机模式  允许设置
}

void NRF24L01_TX_Mode(void)   //配置为发送模式
{
   NRF24L01_CE=0;
   NRF24L01_Write_Buf(WRITE_REG_NRF+TX_ADDR,(unsigned char *)TX_ADDRESS,TX_ADR_WIDTH);    //写TX通道地址  写发送通道地址
   NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P0,(unsigned char *)TX_ADDRESS,RX_ADR_WIDTH); //设置TX通道地址,主要为了使能ACK	 必须与发射地址一致
	
	//经测试  只要开启通道0的接收与应答即可     可能原因是  同一时刻只能向一个通道发送
   NRF24L01_Write_Reg(WRITE_REG_NRF+EN_AA,0x01);     //使能通道0的自动应答    
   NRF24L01_Write_Reg(WRITE_REG_NRF+EN_RXADDR,0x01); //使能通道0的接收地址  
	 NRF24L01_Write_Reg(WRITE_REG_NRF+SETUP_RETR,0x00); //自动重发关闭
   NRF24L01_Write_Reg(WRITE_REG_NRF+RF_CH,40);       //设置RF通道为40   发射与接收必须一致
   NRF24L01_Write_Reg(WRITE_REG_NRF+RF_SETUP,0x0f);  //设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
   NRF24L01_Write_Reg(WRITE_REG_NRF+CONFIG,0x0e);    //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式,开启所有中断  上电  自动CRC
   NRF24L01_CE=1; 
}

//启动NRF24L01发送一次数据
//txbuf:待发送数据首地址
//返回值:发送完成状况
unsigned char  NRF24L01_TxPacket(unsigned char  *txbuf)
{
	unsigned char sta; 
	NRF24L01_CE=0;     //使能拉低
  NRF24L01_Write_Buf(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);//写数据到TX BUF  32个字节
 	NRF24L01_CE=1;//启动发送	   
	
	while(NRF24L01_IRQ!=0);//等待发送完成
	
	sta=NRF24L01_Read_Reg(STATUS);  //读取状态寄存器的值	   
	NRF24L01_Write_Reg(WRITE_REG_NRF+STATUS,sta); //清除TX_DS或MAX_RT中断标志
	if(sta&MAX_TX)//达到最大重发次数
	{
		NRF24L01_Write_Reg(FLUSH_TX,0xff);//清除TX FIFO寄存器 
		return MAX_TX; 
	}
	if(sta&TX_OK)//发送完成
	{
		return TX_OK;
	}
	return 0xff;//其他原因发送失败
}

//指定接收通道地址发送数据
//num 0~5 对应接收通道6个地址  大于5为发送地址
// txbuf  发送数据缓存  最大32字节
//返回值：发送完成状态  TX_OK成功
unsigned char  NRF24L01_TxPacket_NUM(unsigned char  *txbuf,unsigned char  num)
{
	NRF24L01_CE=0;
	
  if(num<=5)  //接收通道地址发送
	{
   NRF24L01_Write_Buf(WRITE_REG_NRF+TX_ADDR,(unsigned char *)RX_ADDRESS[num],TX_ADR_WIDTH);    //写TX通道地址  写发送通道地址
   NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P0,(unsigned char *)RX_ADDRESS[num],RX_ADR_WIDTH); //设置TX通道地址,主要为了使能ACK	 必须与发射地址一致
  }
	else
	{
	 NRF24L01_Write_Buf(WRITE_REG_NRF+TX_ADDR,(unsigned char *)TX_ADDRESS,TX_ADR_WIDTH);    //写TX通道地址  写发送通道地址
   NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P0,(unsigned char *)TX_ADDRESS,RX_ADR_WIDTH); //设置TX通道地址,主要为了使能ACK	 必须与发射地址一致
	}
	NRF24L01_CE=1;
	
 return NRF24L01_TxPacket(txbuf);
}


//该函数初始化NRF24L01到RX模式  设置为接收模式
//设置RX地址,写RX数据宽度,选择RF频道,波特率和LNA HCURR
//当CE变高后,即进入RX模式,并可以接收数据了		   
void NRF24L01_RX_Mode(unsigned char  num)
{
	//注意！P0通道 5字节地址完全独立！  P1通道5字节地址完全独立！  
	// P2~P5通道 低八位独立  高字节与P1完全相同！
	NRF24L01_CE=0;	  
  	NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P0,(unsigned char *)RX_ADDRESS[num],RX_ADR_WIDTH);//写RX通道地址 第0个接收接收通道   地址独立
	
//  	NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P1,(unsigned char *)RX_ADDRESS[1],RX_ADR_WIDTH);//写RX通道地址  这里非常重要！！第1个接收通道要写5位！  地址独立
	
	//通道2~5低8位独立   只需写一个字节    高位与通道1地址相同-------------
 // 	NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P2,(unsigned char *)RX_ADDRESS[2],1);
 //	NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P3,(unsigned char *)RX_ADDRESS[3],1);
 //	NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P4,(unsigned char *)RX_ADDRESS[4],1);
 // 	NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P5,(unsigned char *)RX_ADDRESS[5],1);
	
	//全为32字节接收宽度-------------------------------------
	NRF24L01_Write_Reg(WRITE_REG_NRF+RX_PW_P0,RX_PLOAD_WIDTH);//选择通道0的有效数据宽度
//	NRF24L01_Write_Reg(WRITE_REG_NRF+RX_PW_P1,RX_PLOAD_WIDTH);//选择通道1的有效数据宽度 	
//	NRF24L01_Write_Reg(WRITE_REG_NRF+RX_PW_P2,RX_PLOAD_WIDTH);//选择通道2的有效数据宽度 	
//	NRF24L01_Write_Reg(WRITE_REG_NRF+RX_PW_P3,RX_PLOAD_WIDTH);//选择通道3的有效数据宽度 	
//	NRF24L01_Write_Reg(WRITE_REG_NRF+RX_PW_P4,RX_PLOAD_WIDTH);//选择通道4的有效数据宽度 	
//	NRF24L01_Write_Reg(WRITE_REG_NRF+RX_PW_P5,RX_PLOAD_WIDTH);//选择通道5的有效数据宽度 	
		
	//使能所以通道接收-------------------------------------------------	
  	NRF24L01_Write_Reg(WRITE_REG_NRF+EN_AA,0x3f);    //使能通道0~5的自动应答   	
  	NRF24L01_Write_Reg(WRITE_REG_NRF+EN_RXADDR,0x3f);//使能通道0~5的接收地址  	 
  	NRF24L01_Write_Reg(WRITE_REG_NRF+RF_CH,40);	     //设置RF通信频率
		
				
  	NRF24L01_Write_Reg(WRITE_REG_NRF+RF_SETUP,0x0f);//设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
  	NRF24L01_Write_Reg(WRITE_REG_NRF+CONFIG, 0x0f);//配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式 
  	NRF24L01_CE = 1; //CE为高,进入接收模式 
}	

//启动NRF24L01接收一次数据
//txbuf:接收缓存数据首地址
//返回值:0，接收完成；其他，错误代码
// unsigned char  NRF24L01_RxPacket(unsigned char  *rxbuf,unsigned char  *num)
unsigned char  NRF24L01_RxPacket(unsigned char  *rxbuf)
{
	unsigned char  sta;		 

		sta=NRF24L01_Read_Reg(STATUS);  //读取状态寄存器的值    	 
		NRF24L01_Write_Reg(WRITE_REG_NRF+STATUS,sta); //清除TX_DS或MAX_RT中断标志
		if(sta&RX_OK)//接收到数据
		{
			NRF24L01_Read_Buf(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);//读取数据
			NRF24L01_Write_Reg(FLUSH_RX,0xff);//清除RX FIFO寄存器 
			return 0; 
		}	   
		return 1;//没收到任何数据
}	
