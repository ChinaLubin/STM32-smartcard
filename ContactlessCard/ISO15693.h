
#ifndef _ISO15693_H
#define _ISO15693_H


/*
�ļ���;:			ISO15693Э��
����:					�Ŷ���
����ʱ��:			2018/10/22
����ʱ��:			2018/10/22
�汾:					V1.1

*/



#include "stm32f10x.h"
#include "THM3070.h"


/*��������ֵ������ձ�

	//���к���0x00����ɹ�,��0x00����ʧ��
	0x00	ִ�гɹ�
	0x04	�������ݳ�ʱ

	//����Ӧ�Ĵ�����,��ISO15693����
	0x01	��֧������,�������벻�ܱ�ʶ��
	0x02	����ܱ�ʶ��,��:����һ�θ�ʽ����
	0x03	��֧������ѡ��
	0x0F	�����벻֧�ָô���
	0x10	�涨�鲻����
	0x11	�涨�鱻��,��˲�������
	0x12	�涨�鱻��,���ݲ��ܱ��ı�
	0x13	�涨��û�б��ɹ����
	0x14	�涨��û�б��ɹ�����


*/



uint8_t FINDV(uint16_t ENAndAFI,uint8_t* DAT_UID);															//����

uint8_t Inventory(uint16_t ENAndAFI,uint8_t* DAT_UID);													//���
uint8_t Stayquiet(void);																												//��Ĭ
uint8_t Select(void);																														//ѡ��
uint8_t ResetToReady(void);																											//׼��

uint8_t ReadBlocks(uint8_t BlockNum,uint8_t* BlockData,uint16_t* BlockDataLen);	//����
uint8_t WriteBlocks(uint8_t BlockNum,uint8_t* BlockData,uint16_t BlockDataLen);	//д��
uint8_t ReadMultipleBlocks(uint8_t BlockNum,uint8_t BlockLen,uint8_t* BlockData,uint16_t* BlockDataLen);	//�������
uint8_t WriteMultipleBlocks(uint8_t BlockNum,uint8_t BlockLen,uint8_t* BlockData,uint16_t BlockDataLen);	//д�����

uint8_t WriteAFI(uint8_t AFI);																									//дAFI
uint8_t WriteDSFID(uint8_t DSFID);																							//дDSFID

uint8_t ReadSysInfo(uint8_t* InfoData,uint16_t* InfoDataLen);										//��ȡϵͳ��Ϣ
uint8_t ReadMultipleStatus(uint8_t BlockNum,uint8_t BlockLen,uint8_t* Status,uint16_t* StatusLen);				//�������״̬

uint8_t SendRFUCMD(uint8_t* SendData,uint16_t SendDataLen,uint8_t* RecvData,uint16_t* RecvDataLen);				//͸������

uint8_t TESTV(void);																														//���Կ�Ƭ�Ƿ��������߳���



uint8_t LockBlocks(uint8_t BlockNum);																						//����������,����
uint8_t LockAFI(void);																													//��������AFI,����
uint8_t LockDSFID(void);																												//��������DSFID,����

#endif

