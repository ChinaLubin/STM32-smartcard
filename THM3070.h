
#ifndef _THM3070_H
#define _THM3070_H

/*
�ļ���;:			THM3070�����ļ�
����:					�Ŷ���
����ʱ��:			2018/04/20
����ʱ��:			2018/05/31
�汾:					V1.1

*/


#include "stm32f10x.h"


#define THM_SPIMOD					1																	//1Ӳ��SPI,0���ģ��SPI

#if THM_SPIMOD==1
#define THM_SPIx						SPI2
#endif
/*���Ŷ���*/
//SPIͨ�Žӿ�
#define THM_IO1_SCK_GPIOx		GPIOB
#define THM_IO1_SCK_Pinx		GPIO_Pin_13
#define THM_IO2_MOSI_GPIOx	GPIOB
#define THM_IO2_MOSI_Pinx		GPIO_Pin_15
#define THM_IO3_MISO_GPIOx	GPIOB
#define THM_IO3_MISO_Pinx		GPIO_Pin_14
#define THM_IO4_SS_N_GPIOx	GPIOC
#define THM_IO4_SS_N_Pinx		GPIO_Pin_6

//�ӿ�ģʽ���͹��ġ���λ��
#define THM_MOD0_GPIOx			GPIOC
#define THM_MOD0_Pinx				GPIO_Pin_7
#define THM_STANDBY_GPIOx		GPIOA
#define THM_STANDBY_Pinx		GPIO_Pin_8
#define THM_RESET_GPIOx			GPIOC
#define THM_RESET_Pinx			GPIO_Pin_8

//ʱ��,�ǵð�ʹ�õ�����ʱ�Ӷ�д��
#define THM_RCC_APBxPeriphClockCmd	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO,ENABLE);\
																		RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE)
/*���Ŷ���*/


/*�Ĵ�������*/
#define THM_REG_PSEL		0x01																	//Э��ѡ��
#define THM_REG_FCONB		0x02																	//TYPEBЭ��֡����
#define THM_REG_EGT			0x03																	//TYPEBЭ��EGT����
#define THM_REG_CRCSEL	0x04																	//CRC����
#define THM_REG_INTCON	0x07																	//�жϿ���
#define THM_REG_SMOD		0x10																	//ISO/IEC15693����ģʽ�趨
#define THM_REG_PWYH		0x11																	//����������
#define THM_REG_STATCTRL 0x12																	//��֪����ɶ
#define THM_REG_FMCTRL	0x13																	//֡��ʽ����
#define THM_REG_EMVEN		0x20																	//������⼰����ʹ��
#define THM_REG_TXCON		0x40																	//��Ƶ���͵�·����
#define THM_REG_RXCON		0x45																	//��Ƶ���յ�·����

#define THM_REG_DATA		0x00																	//���ݼĴ���
#define THM_REG_RSTAT		0x05																	//����״̬�Ĵ���
#define THM_REG_SCON		0x06																	//���Ϳ��ƼĴ���
#define THM_REG_RSCH		0x08																	//���ͽ��ռ��������ֽ�
#define THM_REG_RSCL		0x09																	//���ͽ��ռ��������ֽ�
#define THM_REG_CRCH		0x0A																	//CRC������ֽ�
#define THM_REG_CRCL		0x0B																	//CRC������ֽ�
#define THM_REG_BITPOS	0x0E																	//��ͻ����λ
#define THM_REG_EMVERR	0x25																	//������FDT����״̬
#define THM_REG_TXFIN		0x26																	//�������״̬

#define THM_REG_TMRH		0x0C																	//���ն�ʱ�����ֽ�
#define THM_REG_TMRL		0x0D																	//���ն�ʱ�����ֽ�
#define THM_REG_FWIHIGH	0x21																	//FWI��λ
#define THM_REG_FWIMID	0x22																	//FWI��λ
#define THM_REG_FWILOW	0x23																	//FWI��λ

#define THM_REG_AFDTOFFSET	0x24															//TYPEA_FDT�߽�ֵ
#define THM_REG_TR0MINH	0x2E																	//TR0min��λ
#define THM_REG_TR0MINL	0x2F																	//TR0min��λ
#define THM_REG_TR1MINH	0x33																	//TR1min��λ
#define THM_REG_TR1MINL	0x34																	//TR1min��λ
#define THM_REG_TR1MAXH	0x35																	//TR1max��λ
#define THM_REG_TR1MAXL	0x36																	//TR1max��λ
#define THM_REG_TXDP1		0x41																	//PMOS�����ߵ�ƽ�������
#define THM_REG_TXDP0		0x42																	//PMOS�����͵�ƽ�������
#define THM_REG_TXDN1		0x43																	//NMOS�����ߵ�ƽ�������
#define THM_REG_TXDN0		0x44																	//NMOS�����͵�ƽ�������
#define THM_REG_RNGCON	0x30																	//���������
#define THM_REG_RNGSTS	0x31																	//�����״̬
#define THM_REG_RNGDATA	0x32																	//���������
/*�Ĵ�������*/


/*���մ����붨��*/
#define THM_RSTST_OTHER					0x80													//��������
#define THM_RSTST_CERR					0x40													//��������ײ
#define THM_RSTST_PERR					0x20													//��żУ�����
#define THM_RSTST_FERR					0x10													//֡��ʽ����
#define THM_RSTST_DATOVER				0x08													//���������������
#define THM_RSTST_TMROVER				0x04													//���ճ�ʱ
#define THM_RSTST_CRCERR				0x02													//CRC����
#define THM_RSTST_FEND					0x00													//�������
/*���մ����붨��*/


#define THM_ERROR_EMVERR				0x25



/*�ⲿ��������*/
uint8_t	THM3070_Init(void);																		//оƬ��ʼ��

void		THM3070_PortMode(uint8_t mode);												//�ӿ�ģʽ(SPI/IDR)
void		THM3070_SleepMode(uint8_t mode);											//���л�͹���ģʽ

void		THM3070_RFReset(void);																//�س��ٿ���
void		THM3070_RFClose(void);																//�س�

void    THM3070_SetFWT(uint32_t fwt);													//���ó�ʱʱ��
void    THM3070_SetTYPEA(void);																//����TYPEAģʽ
void    THM3070_SetTYPEB(void);																//����TYPEBģʽ
void    THM3070_SetTxBaud(uint8_t baud);											//���÷�������
void    THM3070_SetRxBaud(uint8_t baud);											//���ý�������

void 		THM3070_WriteREG(uint8_t addr,uint8_t val);						//д�Ĵ���
uint8_t THM3070_ReadREG(uint8_t addr);												//���Ĵ���

void		THM3070_SendFrame(uint8_t* p_buff,uint16_t len_buff);	//����֡
uint8_t	THM3070_RecvFrame(uint8_t* p_buff,uint16_t* len_buff);//����֡


void    THM3070_SetMIFARE(void);															//����MIFAREģʽ
void    THM3070_SetFrameFormat(uint8_t format);									//����֡��ʽ
void		THM3070_SendFrame_M(uint8_t* p_buff,uint16_t len_buff);	//����֡Mifare
uint8_t	THM3070_RecvFrame_M(uint8_t* p_buff,uint16_t* len_buff);//����֡Mifare

/*�ⲿ��������*/


#endif


