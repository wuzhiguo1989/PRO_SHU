//
// nRF24L01.c

#include "nRF24L01.h"
#include "stc15w408as.h"
//=======================Ӳ��SPI��ʼ��=================================================================================================
void SPI_Init(void) //����ʱ�Ӹߵ�ƽ  �ڶ����ز���  ��һλΪ���λ    ���� CPU_CLK/4  
{
   SPSTAT=0XC0;  //���㴫����ɱ�ע��д��ͻ��־
	 SPCTL=SPI_NRF24L01;   //NRF24L01   ����ʱ�ӵ͵�ƽ  ��һ���ز���  ��һλΪ���λ    ���� CPU_CLK/4   δ����SPI
   SPCTL|=SPI_STAT;    //����SPI
}

unsigned char SPI_WRITE_READ(unsigned dat)  //SPI ��д�ֽ�
{
   SPDAT =dat;               //д��Ĵ���
   while(!(SPSTAT & 0X80));  //�ȴ��������
	 SPSTAT=0XC0;       //�����־
	 return SPDAT;
}

void SPI_SWITCH(unsigned char num) //SPI�����л� 0 P12 P13 P14 P15, 1 P24 P23 P22 P21, 2 P54 P40 P41 P43  ������Ч ���ŷֱ��Ӧ  SS MOSI MISO SCLK
{
   	num=num&0x03;        //Լ����Χ
	if(num==3)  return ; //��Ч����
	P_SW1&=0xF3;         //����֮ǰ����
   	P_SW1|= num<<2;
}

void SPI_SPEED(unsigned char num)  //����SPI �ٶ�  0~3  0Ϊ����ٶ�
{
   	num=num&0x03;        //Լ����Χ
	SPCTL&=~0x03;        //����֮ǰ����
   	SPCTL|= num; 
	SPCTL|=SPI_STAT;    //����SPI
}
//===================================================================================================


//���͵�ַ---------------------------------------------------------------------
code unsigned char  TX_ADDRESS[TX_ADR_WIDTH]={0x00,0x43,0x10,0x10,0x01}; //���͵�ַ һ��ֻ����һ����ַ���� 5�ֽڵ�ַ����  ����ʱ������RX_ADDR_P0��ַ��ͬ

//����ͨ����ַ-------------------------------------------------------------------------
code unsigned char  RX_ADDRESS[6][5]={                //6������ͨ��     ÿ��ͨ��5���ֽ�  ��0�͵�1ͨ����ַ��ȫ���� ��2~5ͨ����8λ���� ��λ��ͨ��1��ͬ��
													{0x00,0x43,0x10,0x10,0x01},   //5�ֽڵ�ַ��ȫ����

													{0x01,0x43,0x10,0x10,0x01},   //����ͨ��1��ַ  5�ֽڵ�ַ��ȫ����
													{0x02,0x43,0x10,0x10,0x01},   //����ͨ��2��ַ  ��8λ��ַ����  ��λ��ͨ��1��ͬ
													{0x03,0x43,0x10,0x10,0x01},   //����ͨ��3��ַ  ��8λ��ַ����  ��λ��ͨ��1��ͬ
													{0x04,0x43,0x10,0x10,0x01},   //����ͨ��4��ַ  ��8λ��ַ����  ��λ��ͨ��1��ͬ
													{0x05,0x43,0x10,0x10,0x01},   //����ͨ��5��ַ  ��8λ��ַ����  ��λ��ͨ��1��ͬ
                        };


//SPIд�Ĵ���
//reg:ָ���Ĵ�����ַ
//value:д���ֵ
unsigned char  NRF24L01_Write_Reg(unsigned char  reg,unsigned char  value)
{
	  unsigned char  status;	
   	NRF24L01_CSN=0;                 //ʹ��SPI����
  	status =SPI_WRITE_READ(reg);	//���ͼĴ����� 
  	SPI_WRITE_READ(value);      	//д��Ĵ�����ֵ
  	NRF24L01_CSN=1;                 //��ֹSPI����	   
  	return(status);       			//����״ֵ̬
}

//��ȡSPI�Ĵ���ֵ
//reg:Ҫ���ļĴ���
unsigned char  NRF24L01_Read_Reg(unsigned char  reg)
{
	unsigned char  reg_val;	    
 	NRF24L01_CSN = 0;          //ʹ��SPI����		
  SPI_WRITE_READ(reg);   //���ͼĴ�����
  reg_val=SPI_WRITE_READ(0XFF);//��ȡ�Ĵ�������
  NRF24L01_CSN = 1;          //��ֹSPI����		    
  return(reg_val);           //����״ֵ̬
}	

//��ָ��λ�ö���ָ�����ȵ�����
//reg:�Ĵ���(λ��)
//*pBuf:����ָ��
//len:���ݳ���
//����ֵ,�˴ζ�����״̬�Ĵ���ֵ 
unsigned char  NRF24L01_Read_Buf(unsigned char  reg,unsigned char  *pBuf,unsigned char  len)
{
	  unsigned char  status,u8_ctr;	       
  	NRF24L01_CSN = 0;           //ʹ��SPI����
  	status=SPI_WRITE_READ(reg);//���ͼĴ���ֵ(λ��),����ȡ״ֵ̬   	   
 	for(u8_ctr=0;u8_ctr<len;u8_ctr++)pBuf[u8_ctr]=SPI_WRITE_READ(0XFF);//��������
  	NRF24L01_CSN=1;       //�ر�SPI����
  	return status;        //���ض�����״ֵ̬
}

//��ָ��λ��дָ�����ȵ�����
//reg:�Ĵ���(λ��)
//*pBuf:����ָ��
//len:���ݳ���
//����ֵ,�˴ζ�����״̬�Ĵ���ֵ
unsigned char  NRF24L01_Write_Buf(unsigned char  reg, unsigned char  *pBuf, unsigned char  len)
{
	unsigned char  status,u8_ctr;	    
 	NRF24L01_CSN = 0;          //ʹ��SPI����
  status = SPI_WRITE_READ(reg);//���ͼĴ���ֵ(λ��),����ȡ״ֵ̬
  for(u8_ctr=0; u8_ctr<len; u8_ctr++)SPI_WRITE_READ(*pBuf++); //д������	 
  NRF24L01_CSN = 1;       //�ر�SPI����
  return status;          //���ض�����״ֵ̬
}	

void  NRF24L01_Init(void)
{
  SPI_Init();      //��ʼ��SPI
	SPI_SPEED(0);    //Լ���ٶ�   NRF24L01����ٶ�10M
	SPI_SWITCH(0);
  NRF24L01_CSN=1;  //�ر�Ƭѡ
	NRF24L01_IRQ=1;  //�ж�IO ����ߵ�ƽ  ��������Ч��
	NRF24L01_CE=0;   //�������ģʽ  ��������
}

void NRF24L01_TX_Mode(void)   //����Ϊ����ģʽ
{
   NRF24L01_CE=0;
   NRF24L01_Write_Buf(WRITE_REG_NRF+TX_ADDR,(unsigned char *)TX_ADDRESS,TX_ADR_WIDTH);    //дTXͨ����ַ  д����ͨ����ַ
   NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P0,(unsigned char *)TX_ADDRESS,RX_ADR_WIDTH); //����TXͨ����ַ,��ҪΪ��ʹ��ACK	 �����뷢���ַһ��
	
	//������  ֻҪ����ͨ��0�Ľ�����Ӧ�𼴿�     ����ԭ����  ͬһʱ��ֻ����һ��ͨ������
   NRF24L01_Write_Reg(WRITE_REG_NRF+EN_AA,0x01);     //ʹ��ͨ��0���Զ�Ӧ��    
   NRF24L01_Write_Reg(WRITE_REG_NRF+EN_RXADDR,0x01); //ʹ��ͨ��0�Ľ��յ�ַ  
	 NRF24L01_Write_Reg(WRITE_REG_NRF+SETUP_RETR,0x00); //�Զ��ط��ر�
   NRF24L01_Write_Reg(WRITE_REG_NRF+RF_CH,40);       //����RFͨ��Ϊ40   ��������ձ���һ��
   NRF24L01_Write_Reg(WRITE_REG_NRF+RF_SETUP,0x0f);  //����TX�������,0db����,2Mbps,���������濪��   
   NRF24L01_Write_Reg(WRITE_REG_NRF+CONFIG,0x0e);    //���û�������ģʽ�Ĳ���;PWR_UP,EN_CRC,16BIT_CRC,����ģʽ,���������ж�  �ϵ�  �Զ�CRC
   NRF24L01_CE=1; 
}

//����NRF24L01����һ������
//txbuf:�����������׵�ַ
//����ֵ:�������״��
unsigned char  NRF24L01_TxPacket(unsigned char  *txbuf)
{
	unsigned char sta; 
	NRF24L01_CE=0;     //ʹ������
  NRF24L01_Write_Buf(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);//д���ݵ�TX BUF  32���ֽ�
 	NRF24L01_CE=1;//��������	   
	
	while(NRF24L01_IRQ!=0);//�ȴ��������
	
	sta=NRF24L01_Read_Reg(STATUS);  //��ȡ״̬�Ĵ�����ֵ	   
	NRF24L01_Write_Reg(WRITE_REG_NRF+STATUS,sta); //���TX_DS��MAX_RT�жϱ�־
	if(sta&MAX_TX)//�ﵽ����ط�����
	{
		NRF24L01_Write_Reg(FLUSH_TX,0xff);//���TX FIFO�Ĵ��� 
		return MAX_TX; 
	}
	if(sta&TX_OK)//�������
	{
		return TX_OK;
	}
	return 0xff;//����ԭ����ʧ��
}

//ָ������ͨ����ַ��������
//num 0~5 ��Ӧ����ͨ��6����ַ  ����5Ϊ���͵�ַ
// txbuf  �������ݻ���  ���32�ֽ�
//����ֵ���������״̬  TX_OK�ɹ�
unsigned char  NRF24L01_TxPacket_NUM(unsigned char  *txbuf,unsigned char  num)
{
	NRF24L01_CE=0;
	
  if(num<=5)  //����ͨ����ַ����
	{
   NRF24L01_Write_Buf(WRITE_REG_NRF+TX_ADDR,(unsigned char *)RX_ADDRESS[num],TX_ADR_WIDTH);    //дTXͨ����ַ  д����ͨ����ַ
   NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P0,(unsigned char *)RX_ADDRESS[num],RX_ADR_WIDTH); //����TXͨ����ַ,��ҪΪ��ʹ��ACK	 �����뷢���ַһ��
  }
	else
	{
	 NRF24L01_Write_Buf(WRITE_REG_NRF+TX_ADDR,(unsigned char *)TX_ADDRESS,TX_ADR_WIDTH);    //дTXͨ����ַ  д����ͨ����ַ
   NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P0,(unsigned char *)TX_ADDRESS,RX_ADR_WIDTH); //����TXͨ����ַ,��ҪΪ��ʹ��ACK	 �����뷢���ַһ��
	}
	NRF24L01_CE=1;
	
 return NRF24L01_TxPacket(txbuf);
}


//�ú�����ʼ��NRF24L01��RXģʽ  ����Ϊ����ģʽ
//����RX��ַ,дRX���ݿ��,ѡ��RFƵ��,�����ʺ�LNA HCURR
//��CE��ߺ�,������RXģʽ,�����Խ���������		   
void NRF24L01_RX_Mode(unsigned char  num)
{
	//ע�⣡P0ͨ�� 5�ֽڵ�ַ��ȫ������  P1ͨ��5�ֽڵ�ַ��ȫ������  
	// P2~P5ͨ�� �Ͱ�λ����  ���ֽ���P1��ȫ��ͬ��
	NRF24L01_CE=0;	  
  	NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P0,(unsigned char *)RX_ADDRESS[num],RX_ADR_WIDTH);//дRXͨ����ַ ��0�����ս���ͨ��   ��ַ����
	
//  	NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P1,(unsigned char *)RX_ADDRESS[1],RX_ADR_WIDTH);//дRXͨ����ַ  ����ǳ���Ҫ������1������ͨ��Ҫд5λ��  ��ַ����
	
	//ͨ��2~5��8λ����   ֻ��дһ���ֽ�    ��λ��ͨ��1��ַ��ͬ-------------
 // 	NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P2,(unsigned char *)RX_ADDRESS[2],1);
 //	NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P3,(unsigned char *)RX_ADDRESS[3],1);
 //	NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P4,(unsigned char *)RX_ADDRESS[4],1);
 // 	NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P5,(unsigned char *)RX_ADDRESS[5],1);
	
	//ȫΪ32�ֽڽ��տ��-------------------------------------
	NRF24L01_Write_Reg(WRITE_REG_NRF+RX_PW_P0,RX_PLOAD_WIDTH);//ѡ��ͨ��0����Ч���ݿ��
//	NRF24L01_Write_Reg(WRITE_REG_NRF+RX_PW_P1,RX_PLOAD_WIDTH);//ѡ��ͨ��1����Ч���ݿ�� 	
//	NRF24L01_Write_Reg(WRITE_REG_NRF+RX_PW_P2,RX_PLOAD_WIDTH);//ѡ��ͨ��2����Ч���ݿ�� 	
//	NRF24L01_Write_Reg(WRITE_REG_NRF+RX_PW_P3,RX_PLOAD_WIDTH);//ѡ��ͨ��3����Ч���ݿ�� 	
//	NRF24L01_Write_Reg(WRITE_REG_NRF+RX_PW_P4,RX_PLOAD_WIDTH);//ѡ��ͨ��4����Ч���ݿ�� 	
//	NRF24L01_Write_Reg(WRITE_REG_NRF+RX_PW_P5,RX_PLOAD_WIDTH);//ѡ��ͨ��5����Ч���ݿ�� 	
		
	//ʹ������ͨ������-------------------------------------------------	
  	NRF24L01_Write_Reg(WRITE_REG_NRF+EN_AA,0x3f);    //ʹ��ͨ��0~5���Զ�Ӧ��   	
  	NRF24L01_Write_Reg(WRITE_REG_NRF+EN_RXADDR,0x3f);//ʹ��ͨ��0~5�Ľ��յ�ַ  	 
  	NRF24L01_Write_Reg(WRITE_REG_NRF+RF_CH,40);	     //����RFͨ��Ƶ��
		
				
  	NRF24L01_Write_Reg(WRITE_REG_NRF+RF_SETUP,0x0f);//����TX�������,0db����,2Mbps,���������濪��   
  	NRF24L01_Write_Reg(WRITE_REG_NRF+CONFIG, 0x0f);//���û�������ģʽ�Ĳ���;PWR_UP,EN_CRC,16BIT_CRC,����ģʽ 
  	NRF24L01_CE = 1; //CEΪ��,�������ģʽ 
}	

//����NRF24L01����һ������
//txbuf:���ջ��������׵�ַ
//����ֵ:0��������ɣ��������������
// unsigned char  NRF24L01_RxPacket(unsigned char  *rxbuf,unsigned char  *num)
unsigned char  NRF24L01_RxPacket(unsigned char  *rxbuf)
{
	unsigned char  sta;		 

		sta=NRF24L01_Read_Reg(STATUS);  //��ȡ״̬�Ĵ�����ֵ    	 
		NRF24L01_Write_Reg(WRITE_REG_NRF+STATUS,sta); //���TX_DS��MAX_RT�жϱ�־
		if(sta&RX_OK)//���յ�����
		{
			NRF24L01_Read_Buf(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);//��ȡ����
			NRF24L01_Write_Reg(FLUSH_RX,0xff);//���RX FIFO�Ĵ��� 
			return 0; 
		}	   
		return 1;//û�յ��κ�����
}	
