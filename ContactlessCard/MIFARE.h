
#ifndef _MIFARE_H
#define _MIFARE_H


/*
�ļ���;:			MIFAREЭ��
����:					�Ŷ���
����ʱ��:			2018/10/22
����ʱ��:			2018/10/22
�汾:					V1.1

*/



#include "stm32f10x.h"
#include "THM3070.h"




uint8_t FINDM(uint8_t* DAT_UID,uint16_t* LEN_UID);																			//TYPEA����+����ͻ+ѡ��,����M1��

uint8_t AuthKeyA(uint8_t BlockNum,uint8_t* KeyA);																				//��֤KeyA
uint8_t AuthKeyB(uint8_t BlockNum,uint8_t* KeyB);																				//��֤KeyB

uint8_t ReadBlock(uint8_t BlockNum,uint8_t* BlockData);																	//��ȡ������
uint8_t WriteBlock(uint8_t BlockNum,uint8_t* BlockData);																//д�������

uint8_t ReadValue(uint8_t BlockNum,uint8_t* Value);																			//��ȡֵ
uint8_t WriteValue(uint8_t BlockNum,uint8_t* Value);																		//д��ֵ
uint8_t AddValue(uint8_t BlockNum,uint8_t* Value);																			//��ֵ,�Զ�ת�浽��ǰ��
uint8_t SubValue(uint8_t BlockNum,uint8_t* Value);																			//��ֵ,�Զ�ת�浽��ǰ��

uint8_t TESTM(void);																																		//���Կ�Ƭ�Ƿ������߳���,��δʵ��


uint8_t Decrement(uint8_t BlockNum,uint8_t* Value);																			//��ֵ,һ�����һ��ת��,һ�㲻��
uint8_t Increment(uint8_t BlockNum,uint8_t* Value);																			//��ֵ,һ�����һ��ת��,һ�㲻��
uint8_t Transfre(uint8_t BlockNum);																											//ת��,һ�㲻��

uint8_t Restore(uint8_t BlockNum);																											//�ָ�,������



#endif

