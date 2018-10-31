
#ifndef _ISO14443_H
#define _ISO14443_H


/*
�ļ���;:			ISO14443Э��
����:					�Ŷ���
����ʱ��:			2018/04/20
����ʱ��:			2018/05/31
�汾:					V1.1

*/



#include "stm32f10x.h"
#include "THM3070.h"

/*����ִ��״̬*/
#define ISO_RSTST_CERR					THM_RSTST_CERR																					//��������ײ
#define ISO_RSTST_PERR					THM_RSTST_PERR																					//��żУ�����
#define ISO_RSTST_FERR					THM_RSTST_FERR																					//֡��ʽ����
#define ISO_RSTST_DATOVER				THM_RSTST_DATOVER																				//���������������
#define ISO_RSTST_TMROVER				THM_RSTST_TMROVER																				//���ճ�ʱ
#define ISO_RSTST_CRCERR				THM_RSTST_CRCERR																				//CRC����
#define ISO_RSTST_FEND					THM_RSTST_FEND																					//�������




extern uint8_t ISO_PICC_CIDSUP;																													//���Ƿ�֧��CID,Ĭ��֧��
extern uint32_t ISO_PICC_FWT;																														//ͨ�ŵȴ���ʱʱ��,Ĭ��5ms
extern uint16_t ISO_PICC_MFSIZE;																												//���ܽ��յ����֡��,Ĭ��16



uint8_t REQB(uint8_t slotNum,uint8_t* DAT_ATQB,uint16_t* LEN_ATQB);											//TYPEBѰ��
uint8_t WUPB(uint8_t slotNum,uint8_t* DAT_ATQB,uint16_t* LEN_ATQB);											//TYPEB����
uint8_t SlotMARKER(uint8_t slotIndex,uint8_t* DAT_ATQB,uint16_t* LEN_ATQB);							//TYPEB����ʱ���
uint8_t ATTRIB(uint8_t* DAT_ATTRIBAnswer,uint16_t* LEN_ATTRIBAnswer);										//TYPEB����

uint8_t FINDB(uint8_t* DAT_ATQB,uint16_t* LEN_ATQB);																		//TYPEB����+����
uint8_t TESTB(void);																																		//����TYPEB���Ƿ��ڳ���


uint8_t REQA(uint8_t* DAT_ATQA,uint16_t* LEN_ATQA);																			//TYPEAѰ��
uint8_t WUPA(uint8_t* DAT_ATQA,uint16_t* LEN_ATQA);																			//TYPEA����
uint8_t AnticollAndSelect(uint8_t* DAT_UID,uint16_t* LEN_UID);													//TYPEA����ͻ��ѡ��
uint8_t RATS(uint8_t* DAT_ATS,uint16_t* LEN_ATS);																				//TYPEA����
uint8_t PPSS(uint8_t TxBuad,uint8_t RxBuad);																						//TYPEA��PPS

uint8_t FINDA(uint8_t* DAT_ATS,uint16_t* LEN_ATS);																			//TYPEA����+����ͻ+ѡ��+����
uint8_t TESTA(void);																																		//����TYPEA���Ƿ��ڳ���


uint8_t ExchangeAPDU(uint8_t* sData,uint16_t len_sData,uint8_t* rData,uint16_t* len_rData);	//����APDU�����ȡ��Ӧ,����14443Э���Զ�����,����ֻ�ط�һ��
uint8_t ExchangeData(uint8_t* sData,uint16_t len_sData,uint8_t* rData,uint16_t* len_rData);	//����ԭʼ���ݲ���ȡ��Ӧ



#endif

