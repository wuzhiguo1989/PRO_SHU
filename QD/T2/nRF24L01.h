// nRF24L01.h
#ifndef _NRF_24L01_
#define _NRF_24L01_

//==NRF24L01===============================================================================
/*-----------------------------SPI��ؼĴ�����----------------------------------------------*/
// SPCTL ���ƼĴ���   BIT7   BIT6  BIT5   BIT4   BIT3  BIT2 BIT1 BIT0
//                   SSIG   SPEN  DORD   MSTR   CPOL  CPHA SPR1  SPR0
#define SPI_STAT      0X40   //ʹ��SPI
#define SPI_FLASH     0x9C   //����ʱ�Ӹߵ�ƽ  �ڶ����ز���  ��һλΪ���λ    ���� CPU_CLK/4   δ����SPI
#define SPI_NRF24L01  0x90   //����ʱ�ӵ͵�ƽ  ��һ���ز���  ��һλΪ���λ    ���� CPU_CLK/4   δ����SPI

//SPSTAT  ״̬�Ĵ���    BIT7  SPIF   ������ɱ�־    BIT6  д��ͻ��־    ��Ӧλд1����

// SPDAT  ���ݼĴ���
// IE      BIT7  EA       ���жϿ���
// IE2    �ж�����Ĵ���    BIT1  ESPI   ����SPI�ж�λ
// IP2    �ж����ȼ����ƼĴ���  BIT1  PSPI    0������ȼ��ж�     1������ȼ�
// P_SW1/AUXR1   �����л����ƼĴ���1   BIT3 BIT2  SPI_S1 SPI_S0   
// 00  P12 P13 P14 P15   01 P24 P23 P22 P21    10 P54 P40 P41 P43   11��Ч   ���ŷֱ��Ӧ  SS  MOSI  MISO   SCLK

extern void SPI_Init(void);  //����ʱ�Ӹߵ�ƽ  �ڶ����ز���  ��һλΪ���λ    ���� CPU_CLK/4  
extern void SPI_SWITCH(unsigned char num); //SPI�����л� 0 P12 P13 P14 P15, 1 P24 P23 P22 P21, 2 P54 P40 P41 P43  ������Ч ���ŷֱ��Ӧ  SS MOSI MISO SCLK
extern void SPI_SPEED(unsigned char num);  //����SPI �ٶ�  0~3  0Ϊ����ٶ�
extern unsigned char SPI_WRITE_READ(unsigned dat);  //SPI ��д�ֽ�

//SPI�ӿ�------------------  ����ʱ��Ϊ�͵�ƽ  ��һ�������źŲɼ�����  ����ٶ�10M-----------------------
/******************����Ӳ��SPI*************************************
#define MOSI    P13   
#define MISO    P14
#define SCLK    P15
*******************************************************************/

#define NRF24L01_IRQ     P36  // �ж��ź�����  �͵�ƽΪ�ж��ź�
#define NRF24L01_CSN     P12  // SPI ʹ���ź�  Ƭѡ�ź�
#define NRF24L01_CE      P37  // ����ģ��ʹ���ź�   �ߵ�ƽ����ģʽ  �͵�����ģʽ  �����أ�����10us����������    ֻ����CEΪ�͵�ƽʱ����д�Ĵ���

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//NRF24L01�Ĵ�����������
#define READ_REG_NRF    0x00  //�����üĴ���,��5λΪ�Ĵ�����ַ
#define WRITE_REG_NRF   0x20  //д���üĴ���,��5λΪ�Ĵ�����ַ
#define RD_RX_PLOAD     0x61  //��RX��Ч����,1~32�ֽ�
#define WR_TX_PLOAD     0xA0  //дTX��Ч����,1~32�ֽ�
#define FLUSH_TX        0xE1  //���TX FIFO�Ĵ���.����ģʽ����
#define FLUSH_RX        0xE2  //���RX FIFO�Ĵ���.����ģʽ����
#define REUSE_TX_PL     0xE3  //����ʹ����һ������,CEΪ��,���ݰ������Ϸ���.
#define NOP             0xFF  //�ղ���,����������״̬�Ĵ���	 
//SPI(NRF24L01)�Ĵ�����ַ
#define CONFIG          0x00  //���üĴ�����ַ;bit0:1����ģʽ,0����ģʽ;bit1:��ѡ��;bit2:CRCģʽ;bit3:CRCʹ��;
                              //bit4:�ж�MAX_RT(�ﵽ����ط������ж�)ʹ��;bit5:�ж�TX_DSʹ��;bit6:�ж�RX_DRʹ��
#define EN_AA           0x01  //ʹ���Զ�Ӧ����  bit0~5,��Ӧͨ��0~5
#define EN_RXADDR       0x02  //���յ�ַ����,bit0~5,��Ӧͨ��0~5
#define SETUP_AW        0x03  //���õ�ַ���(��������ͨ��):bit1,0:00,3�ֽ�;01,4�ֽ�;02,5�ֽ�;
#define SETUP_RETR      0x04  //�����Զ��ط�;bit3:0,�Զ��ط�������;bit7:4,�Զ��ط���ʱ 250*x+86us
#define RF_CH           0x05  //RFͨ��,bit6:0,����ͨ��Ƶ��;
#define RF_SETUP        0x06  //RF�Ĵ���;bit3:��������(0:1Mbps,1:2Mbps);bit2:1,���书��;bit0:�������Ŵ�������
#define STATUS          0x07  //״̬�Ĵ���;bit0:TX FIFO����־;bit3:1,��������ͨ����(���:6);bit4,�ﵽ�����ط�
                              //bit5:���ݷ�������ж�;bit6:���������ж�;
//״̬�б�-------------															
#define MAX_TX  		  0x10  //�ﵽ����ʹ����ж�
#define TX_OK   		  0x20  //TX��������ж�
#define RX_OK   		  0x40  //���յ������ж�

#define OBSERVE_TX      0x08  //���ͼ��Ĵ���,bit7:4,���ݰ���ʧ������;bit3:0,�ط�������
#define CD              0x09  //�ز����Ĵ���,bit0,�ز����;
#define RX_ADDR_P0      0x0A  //����ͨ��0���յ�ַ,��󳤶�5���ֽ�,���ֽ���ǰ
#define RX_ADDR_P1      0x0B  //����ͨ��1���յ�ַ,��󳤶�5���ֽ�,���ֽ���ǰ
#define RX_ADDR_P2      0x0C  //����ͨ��2���յ�ַ,����ֽڿ�����,���ֽ�,����ͬRX_ADDR_P1[39:8]���;
#define RX_ADDR_P3      0x0D  //����ͨ��3���յ�ַ,����ֽڿ�����,���ֽ�,����ͬRX_ADDR_P1[39:8]���;
#define RX_ADDR_P4      0x0E  //����ͨ��4���յ�ַ,����ֽڿ�����,���ֽ�,����ͬRX_ADDR_P1[39:8]���;
#define RX_ADDR_P5      0x0F  //����ͨ��5���յ�ַ,����ֽڿ�����,���ֽ�,����ͬRX_ADDR_P1[39:8]���;
#define TX_ADDR         0x10  //���͵�ַ(���ֽ���ǰ),ShockBurstTMģʽ��,RX_ADDR_P0��˵�ַ���
//ͨ��������Ч���ݿ��----------------------------
#define RX_PW_P0        0x11  //��������ͨ��0��Ч���ݿ��(1~32�ֽ�),����Ϊ0��Ƿ�
#define RX_PW_P1        0x12  //��������ͨ��1��Ч���ݿ��(1~32�ֽ�),����Ϊ0��Ƿ�
#define RX_PW_P2        0x13  //��������ͨ��2��Ч���ݿ��(1~32�ֽ�),����Ϊ0��Ƿ�
#define RX_PW_P3        0x14  //��������ͨ��3��Ч���ݿ��(1~32�ֽ�),����Ϊ0��Ƿ�
#define RX_PW_P4        0x15  //��������ͨ��4��Ч���ݿ��(1~32�ֽ�),����Ϊ0��Ƿ�
#define RX_PW_P5        0x16  //��������ͨ��5��Ч���ݿ��(1~32�ֽ�),����Ϊ0��Ƿ�
#define NRF_FIFO_STATUS 0x17  //FIFO״̬�Ĵ���;bit0,RX FIFO�Ĵ����ձ�־;bit1,RX FIFO����־;bit2,3,����
                              //bit4,TX FIFO�ձ�־;bit5,TX FIFO����־;bit6,1,ѭ��������һ���ݰ�.0,��ѭ��;
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//24L01���ͽ������ݿ�ȶ���
#define TX_ADR_WIDTH    5   	//5�ֽڵĵ�ַ���
#define RX_ADR_WIDTH    5   	//5�ֽڵĵ�ַ���
#define TX_PLOAD_WIDTH  2  	//32�ֽڵ��û����ݿ��
#define RX_PLOAD_WIDTH  1  	//32�ֽڵ��û����ݿ��

void  NRF24L01_Init(void);
unsigned char      NRF24L01_Write_Reg(unsigned char   reg,unsigned char   value);          //ָ���Ĵ���д����
unsigned char      NRF24L01_Read_Reg(unsigned char   reg);                    //���Ĵ���
unsigned char      NRF24L01_Read_Buf(unsigned char   reg,unsigned char   *pBuf,unsigned char   len);    //ָ���Ĵ�����ָ������
unsigned char      NRF24L01_Write_Buf(unsigned char   reg, unsigned char   *pBuf, unsigned char   len); //ָ���Ĵ���дָ������
//����----------------------------------------------------------------------
void  NRF24L01_TX_Mode(void);                       //����Ϊ����ģʽ
unsigned char      NRF24L01_TxPacket(unsigned char   *txbuf);                 //����һ������  32�ֽ�
unsigned char      NRF24L01_TxPacket_NUM(unsigned char   *txbuf,unsigned char   num);         //ָ�����յ�ַ����0~5   ����5ΪĬ�Ϸ��͵�ַ����
//����--------------------------------------------------------------------
void  NRF24L01_RX_Mode(unsigned char  num);                       //��Ϊ����ģʽ                      //��Ϊ����ģʽ
// unsigned char      NRF24L01_RxPacket(unsigned char   *rxbuf,unsigned char   *num);         //����һ������ ����״̬  ��ȡͨ��
unsigned char      NRF24L01_RxPacket(unsigned char   *rxbuf);         //����һ������ ����״̬  ��ȡͨ��
#endif