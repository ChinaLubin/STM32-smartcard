
#include "MIFARE.h"


/*
�ļ���;:			MIFAREЭ��
����:					�Ŷ���
����ʱ��:			2018/10/22
����ʱ��:			2018/10/22
�汾:					V1.1

��ʷ�汾:			V1.0:����THM3070ʵ��MIFAREЭ��


*/



#include "string.h"


static uint8_t M1_RecvData[64];
static uint8_t M1_UID[10];

/*
���ܣ�	TYPEAѰ��
����1��	�����Ӧ��ATQA����,��֤��ռ�>=2
����2��	�����Ӧ��ATQA���ݳ���
���أ�	ִ�н��
*/
static uint8_t M_REQA(uint8_t* DAT_ATQA,uint16_t* LEN_ATQA)
{
	uint8_t RSTST;
	
	uint8_t CMD[1]={0x26};														//WUPA����
	
	THM3070_SetMIFARE();
	THM3070_SetFWT(0x05);															//��ʱʱ��Ϊ5*330us=1.65ms	
	
	THM3070_SendFrame_M(CMD,1);													//Э��Ҫ��֡,�������;�ȻҲ����?
	RSTST=THM3070_RecvFrame_M(DAT_ATQA,LEN_ATQA);

	return RSTST;
}

/*
���ܣ�	TYPEA����
����1��	�����Ӧ��ATQA����,��֤��ռ�>=2
����2��	�����Ӧ��ATQA���ݳ���
���أ�	ִ�н��
*/
static uint8_t M_WUPA(uint8_t* DAT_ATQA,uint16_t* LEN_ATQA)
{
	uint8_t RSTST;
	
	uint8_t CMD[1]={0x52};														//WUPA����
	

	THM3070_SetMIFARE();
	THM3070_SetFWT(0x05);															//��ʱʱ��Ϊ5*330us=1.65ms
	
	
	THM3070_SendFrame_M(CMD,1);													//Э��Ҫ��֡,�������;�ȻҲ����?
	RSTST=THM3070_RecvFrame_M(DAT_ATQA,LEN_ATQA);

	return RSTST;
}

/*
���ܣ�	���ͷ���ͻָ����,�ڲ�����
����1��	����ͻ����
����2��	���ص�ѡ��ָ������
����3��	���ص�ѡ��ָ�����ݳ���
���أ�	ִ�н��
*/
static uint8_t M_SendAC(uint8_t casLevel,uint8_t* selCode,uint16_t* Len_selCode)
{
	uint8_t* temp=M1_RecvData+48;
	uint8_t curReceivePostion,lastPostion,RSTST;
	uint16_t len;
	
	temp[0]=casLevel;																					//SEL
	temp[1]=0x20;																							//NVB=0x20,û����֪��UID
	curReceivePostion=lastPostion=0x00;
	
	while(1)
	{
		THM3070_SendFrame_M(temp,curReceivePostion+2);					//
		RSTST=THM3070_RecvFrame_M(temp+lastPostion+2,&len);
		
		curReceivePostion=lastPostion+len;											//�ܹ����յ������ݳ���
		if(len!=0)
		{
			lastPostion+=len-1;																		//ȥ�����1�ֽ�,��Ϊ�����ܴ��г�ͻ
		}
		
		if(RSTST&THM_RSTST_CERR)																//�г�ͻ
		{
			temp[1]=THM3070_ReadREG(THM_REG_BITPOS)+1;						//���յ��ı���λ����,NVB��4λ
			temp[1]+=(uint8_t)(len+1)<<4;													//���յ����ֽڳ���,NVB��4λ
			if((temp[1]&0x0f)==0x08)															//����λ����Ϊ8
			{
				temp[1]=((temp[1]&0xf0)+0x10);											//����λ����,�ֽ�+1
				lastPostion=(lastPostion+1);												//+1
			}
		}
		else if(RSTST==THM_RSTST_FEND||RSTST==THM_RSTST_CRCERR)
		{
			if(lastPostion==4)
			{
				memcpy(selCode+2,temp+2,5);													//û�г�ͻ,����ѡ��ָ������
				*Len_selCode=7;																			//����Ϊ7
				
				return THM_RSTST_FEND;
			}
			else
			{
				return THM_RSTST_OTHER;
			}
		}
		else
		{
			return RSTST;																					//����
		}
	}
}

/*
���ܣ�	TYPEA����ͻ+ѡ��
����1��	��UID,��֤��ռ�>=10
����2��	��UID����
���أ�	ִ�н��
*/
static uint8_t M_AnticollAndSelect(uint8_t* DAT_UID,uint16_t* LEN_UID)
{
	uint8_t* UIDTemp=M1_RecvData;												//�ݴ��յ���UID,���ܰ��������ַ�CT=0x88
	
	uint8_t RSTST,CASLEVEL=0x93;
	uint8_t* selCode=M1_RecvData+32;
	uint16_t len;
	uint8_t i,count;
		
	THM3070_SetFrameFormat(1);													//��׼֡����CRC
	*LEN_UID=0x00;
	
	for(i=0;i<3;i++)
	{
		count=3;
		while(count--)
		{
			RSTST=M_SendAC(CASLEVEL,selCode,&len);					//����SEL=0x93/0x95/0x97
			if(RSTST==THM_RSTST_FEND)
			{
				break;
			}
		}
		if(RSTST==THM_RSTST_FEND)
		{
			memcpy(UIDTemp+i*5,selCode+2,5);								//��ȡ��UID,���ܰ���CT
			if((selCode[0]&0x04)==0x00)											//
			{
				THM3070_SetFrameFormat(2);										//��׼֡��CRC
			}
		}
		else
		{
			return RSTST;
		}
		
		count=3;
		while(count--)
		{
			selCode[0]=CASLEVEL;
			selCode[1]=0x70;																//ѡ��
			THM3070_SendFrame_M(selCode,7);									//
			selCode[0]=0;
			RSTST=THM3070_RecvFrame_M(selCode,&len);				//��ӦSAK
			if(RSTST==THM_RSTST_FEND)
			{
				break;
			}
		}
		if(RSTST==THM_RSTST_FEND)
		{
			if((selCode[0]&0x04)!=0x00)											//SAK��3λΪ1����UID������
			{
				CASLEVEL+=2;																	//������һ��
				memcpy(DAT_UID+i*3,UIDTemp+i*5+1,3);					//��ȡ������UID
			}
			else
			{
				memcpy(DAT_UID+i*3,UIDTemp+i*5,4);						//UID����,��ȡ��UID
				break;
			}
		}
		else
		{
			return RSTST;
		}
	}
	*LEN_UID=4+i*3;																			//UID����4/7/10
	memcpy(M1_UID,DAT_UID,*LEN_UID);										//����UID
	
	return RSTST;
}


/*
���ܣ�	��λ��+TYPEA����+����ͻ+ѡ��
����1��	�����ص�UID,��֤��ռ�>=4*n
����2��	UID�ĳ���
���أ�	ִ�н��
*/
uint8_t FINDM(uint8_t* DAT_UID,uint16_t* LEN_UID)
{
	uint8_t RSTST;
	uint16_t len;
	
	THM3070_RFReset();
		
	RSTST=M_WUPA(M1_RecvData,&len);
	if(RSTST==THM_RSTST_FEND)
	{
		RSTST=M_AnticollAndSelect(M1_RecvData,&len);
		if(RSTST==THM_RSTST_FEND)
		{
			*LEN_UID=len;
			memcpy(DAT_UID,M1_RecvData,len);
		}
	}
	return RSTST;
}

/*
���ܣ�	���Կ�Ƭ�Ƿ������߳���,��δʵ��
����1��	��
���أ�	ִ�н��
*/
uint8_t TESTM()
{

	return THM_RSTST_FEND;
}

/*
���ܣ�	��֤KEY
����1��	KEYA/B,0x60/0x61
����2��	KEY
����3��	���
���أ�	ִ�н��
*/
static uint8_t AuthKey(uint8_t AB,uint8_t* Key,uint8_t BlockNum)
{
	uint8_t i,RSTST;
	uint16_t len;
	uint8_t CMD[2]={0x60,0x00};														//
	CMD[0]=AB;																						//
	CMD[1]=BlockNum;																			//��ֵ���
	
	THM3070_SetFWT(0x64);																	//��ʱʱ��Ϊ100*330us=33ms
	
	THM3070_WriteREG(0x1D,0x00);													//������֤
	
	THM3070_SendFrame_M(CMD,2);
	THM3070_RecvFrame_M(M1_RecvData,&len);
	if(len!=4)																						//����4�ֽ������
	{
		return 2;
	}
	
	THM3070_WriteREG(0x15,0x08);													//
	
	THM3070_WriteREG(0x17,M1_RecvData[0]);								//�����������DATA1-DATA4
	THM3070_WriteREG(0x18,M1_RecvData[1]);
	THM3070_WriteREG(0x19,M1_RecvData[2]);
	THM3070_WriteREG(0x1A,M1_RecvData[3]);
	for(i=0;i<6;i++)																			//����KEYA��DATA0
	{
		THM3070_WriteREG(0x16,Key[i]);
	}
	for(i=0;i<4;i++)																			//����UID��DATA0
	{
		THM3070_WriteREG(0x16,M1_UID[i]);
	}
	THM3070_WriteREG(0x30,0x01);													//���������
	i=0;
	while(1)
	{
		RSTST=THM3070_ReadREG(0x31);
		if(RSTST==1)
		{
			M1_RecvData[i++]=THM3070_ReadREG(0x32);						//��ȡ�����
			if(i>=4)
			{
				break;
			}
		}	
	}
	THM3070_WriteREG(0x17,M1_RecvData[0]);								//��������浽DATA1-4
	THM3070_WriteREG(0x18,M1_RecvData[1]);
	THM3070_WriteREG(0x19,M1_RecvData[2]);
	THM3070_WriteREG(0x1A,M1_RecvData[3]);
	THM3070_WriteREG(0x30,0x00);													//�ر������

	THM3070_WriteREG(0x15,0x0C);													//������֤
	while(1)
	{
		RSTST=THM3070_ReadREG(0x14);												//��ȡ��֤���
		if(RSTST&0xFF)
		{
			break;
		}
	}
	if((RSTST&0xEF)==0x00)																//��֤�ɹ�
	{
		THM3070_WriteREG(0x15,0x08);												//
		THM3070_WriteREG(0x12,0x01);												//֮��ͨ�Ų��ü��ܷ�ʽ
		return 0;
	}
	else if(RSTST&0x80)
	{
		return 3;
	}
	else
	{
		return 4;
	}
}

/*
���ܣ�	��֤KEYA
����1��	KEY
����2��	���
���أ�	ִ�н��
*/
uint8_t AuthKeyA(uint8_t* KeyA,uint8_t BlockNum)
{
	uint8_t RSTST;
	
	RSTST=AuthKey(0x60,KeyA,BlockNum);
	
	return RSTST;
}

/*
���ܣ�	��֤KEYB
����1��	KEY
����2��	���
���أ�	ִ�н��
*/
uint8_t AuthKeyB(uint8_t* KeyB,uint8_t BlockNum)
{
	uint8_t RSTST;
	
	RSTST=AuthKey(0x61,KeyB,BlockNum);
	
	return RSTST;
}

/*
���ܣ�	��ȡ������
����1��	���
����2��	����(16�ֽ�)
���أ�	ִ�н��
*/
uint8_t ReadBlock(uint8_t BlockNum,uint8_t* BlockData)
{
	uint16_t len;
	uint8_t CMD[2]={0x30,0x00};														//������
	CMD[1]=BlockNum;																			//��ֵ���
	
	THM3070_SetFWT(0x64);																	//��ʱʱ��Ϊ100*330us=33ms
	
	THM3070_SendFrame_M(CMD,2);														//��������
	THM3070_RecvFrame_M(M1_RecvData,&len);								//����
	if(len==0x12)
	{
		memcpy(BlockData,M1_RecvData,0x10);									//����
		return 0;
	}
	
	return 1;
}

/*
���ܣ�	д�������
����1��	���
����2��	����(16�ֽ�)
���أ�	ִ�н��
*/
uint8_t WriteBlock(uint8_t BlockNum,uint8_t* BlockData)
{
	uint16_t len;
	uint8_t CMD[2]={0xA0,0x00};														//д����
	CMD[1]=BlockNum;
	
	THM3070_SetFWT(0x64);																	//��ʱʱ��Ϊ100*330us=33ms
	
	THM3070_SendFrame_M(CMD,2);														//��������
	THM3070_RecvFrame_M(M1_RecvData,&len);
	if(len!=0x01||M1_RecvData[0]!=0xA0)										//�����ж�
	{
		return 1;
	}
	THM3070_SendFrame_M(BlockData,0x10);									//��������
	THM3070_RecvFrame_M(M1_RecvData,&len);								//����
	if(len!=0x01||M1_RecvData[0]!=0xA0)										//�����ж�
	{
		return 1;
	}
	
	return 0;
}

/*
���ܣ�	��ȡ��ֵ
����1��	���
����2��	ֵ(4�ֽ�)
���أ�	ִ�н��
*/
uint8_t ReadValue(uint8_t BlockNum,uint8_t* Value)
{
	uint8_t res;
	uint16_t num;
	
	num=BlockNum+1;
	if(num%4==0)return 3;
	
	res=ReadBlock(BlockNum,M1_RecvData);
	if(res==0)
	{
		uint8_t i;
		
		for(i=0;i<4;i++)
		{
			M1_RecvData[i+4]=~M1_RecvData[i+4];								//4-7=~4-7
		}
		for(i=0;i<4;i++)
		{
			if(M1_RecvData[i]!=M1_RecvData[i+4]||M1_RecvData[i]!=M1_RecvData[i+8])//0-3?=~4-7?=8-11
			{
				return 2;
			}
		}
		if(M1_RecvData[12]!=BlockNum||M1_RecvData[14]!=BlockNum)//12=14=BlockNum
		{
			return 2;
		}
		BlockNum=~BlockNum;
		if(M1_RecvData[13]!=BlockNum||M1_RecvData[15]!=BlockNum)//13=15=~BlockNum
		{
			return 2;
		}
		Value[0]=M1_RecvData[3];														//����
		Value[1]=M1_RecvData[2];
		Value[2]=M1_RecvData[1];
		Value[3]=M1_RecvData[0];
		return 0;
	}
	
	return 1;
}

/*
���ܣ�	д���ֵ
����1��	���
����2��	ֵ(4�ֽ�)
���أ�	ִ�н��
*/
uint8_t WriteValue(uint8_t BlockNum,uint8_t* Value)
{
	uint8_t i;
	uint8_t* temp=M1_RecvData+16;
	uint8_t* VTemp=M1_RecvData+40;
	uint16_t num;
	
	num=BlockNum+1;
	if(num%4==0)return 3;
	
	VTemp[0]=Value[3];																	//������
	VTemp[1]=Value[2];
	VTemp[2]=Value[1];
	VTemp[3]=Value[0];

	memcpy(temp,VTemp,0x04);														//0-3=Value
	memcpy(temp+8,VTemp,0x04);													//8-11=value
	for(i=0;i<4;i++)
	{
		temp[i+4]=~VTemp[i];															//4-7=~value
	}
	temp[12]=temp[14]=BlockNum;													//12|14=BlockNum
	temp[13]=temp[15]=~BlockNum;												//13|15=~BlockNum
	
	i=WriteBlock(BlockNum,temp);
	
	return i;
	
}

/*
���ܣ�	��ֵ������浽��ǰ��
����1��	���
����2��	ֵ(4�ֽ�)
���أ�	ִ�н��
*/
uint8_t AddValue(uint8_t BlockNum,uint8_t* Value)
{
	uint8_t res;
	uint16_t num;
	
	num=BlockNum+1;
	if(num%4==0)return 3;
	
	res=Increment(BlockNum,Value);
	if(res!=0)
	{
		return res;
	}
	res=Transfre(BlockNum);
	return res;
}

/*
���ܣ�	��ֵ������浽��ǰ��
����1��	���
����2��	ֵ(4�ֽ�)
���أ�	ִ�н��
*/
uint8_t SubValue(uint8_t BlockNum,uint8_t* Value)
{
	uint8_t res;
	uint16_t num;
	
	num=BlockNum+1;
	if(num%4==0)return 3;
	
	res=Decrement(BlockNum,Value);
	if(res!=0)
	{
		return res;
	}
	res=Transfre(BlockNum);
	return res;
}




/*
���ܣ�	��ֵ(���ٿ��ֵ,������������ڲ��Ĵ�����)
����1��	���
����2��	ֵ(4�ֽ�)
���أ�	ִ�н��
*/
uint8_t Decrement(uint8_t BlockNum,uint8_t* Value)
{
	uint16_t len;
	uint8_t CMD[2]={0xC0,0x00};														//д����
	uint16_t num;
	
	num=BlockNum+1;
	if(num%4==0)return 3;
	
	CMD[1]=BlockNum;
	THM3070_SetFWT(0x64);																	//��ʱʱ��Ϊ100*330us=33ms
	
	THM3070_SendFrame_M(CMD,2);														//��������
	THM3070_RecvFrame_M(M1_RecvData,&len);
	if(len!=0x01||M1_RecvData[0]!=0xA0)										//�����ж�
	{
		return 1;
	}
	
	THM3070_SetFWT(0x64);																	//��ʱʱ��Ϊ100*330us=33ms
	THM3070_SendFrame_M(Value,0x04);											//��������
	len=0;
	THM3070_RecvFrame_M(M1_RecvData,&len);								//����
	if(len>0)																							//Ӧ��û����Ӧ
	{
		return 1;
	}
	
	return 0;
}

/*
���ܣ�	��ֵ(���ӿ��ֵ,������������ڲ��Ĵ�����)
����1��	���
����2��	ֵ(4�ֽ�)
���أ�	ִ�н��
*/
uint8_t Increment(uint8_t BlockNum,uint8_t* Value)
{
	uint16_t len;
	uint8_t CMD[2]={0xC1,0x00};														//д����
	uint16_t num;
	
	num=BlockNum+1;
	if(num%4==0)return 3;
	
	CMD[1]=BlockNum;
	THM3070_SetFWT(0x64);																	//��ʱʱ��Ϊ100*330us=33ms
	
	THM3070_SendFrame_M(CMD,2);														//��������
	THM3070_RecvFrame_M(M1_RecvData,&len);
	if(len!=0x01||M1_RecvData[0]!=0xA0)										//�����ж�
	{
		return 1;
	}
	
	THM3070_SetFWT(0x64);																	//��ʱʱ��Ϊ100*330us=33ms
	THM3070_SendFrame_M(Value,0x04);											//��������
	len=0;
	THM3070_RecvFrame_M(M1_RecvData,&len);								//����
	if(len>0)																							//Ӧ��û����Ӧ
	{
		return 1;
	}
	
	return 0;
}

/*
���ܣ�	ת��(���ڲ��Ĵ�������д�����)
����1��	���
����2��	ֵ(4�ֽ�)
���أ�	ִ�н��
*/
uint8_t Transfre(uint8_t BlockNum)
{
	uint16_t len;
	uint8_t CMD[2]={0xB0,0x00};														//д����
	uint16_t num;
	
	num=BlockNum+1;
	if(num%4==0)return 3;
	
	CMD[1]=BlockNum;
	THM3070_SetFWT(0x64);																	//��ʱʱ��Ϊ100*330us=33ms
		
	THM3070_SendFrame_M(CMD,2);														//��������
	THM3070_RecvFrame_M(M1_RecvData,&len);
	if(len!=0x01||M1_RecvData[0]!=0xA0)										//�����ж�
	{
		return 1;
	}
	return 0;
}

/*
���ܣ�	�ָ�(����������д���ڲ��Ĵ���)
����1��	���
����2��	ֵ(4�ֽ�)
���أ�	ִ�н��
*/
uint8_t Restore(uint8_t BlockNum)
{
	uint16_t len;
	uint8_t CMD[2]={0xC2,0x00};														//д����
	CMD[1]=BlockNum;
	
	THM3070_SetFWT(0x64);																	//��ʱʱ��Ϊ100*330us=33ms
	THM3070_SendFrame_M(CMD,2);														//��������
	THM3070_RecvFrame_M(M1_RecvData,&len);
	if(len!=0x01||M1_RecvData[0]!=0xA0)										//�����ж�
	{
		return 1;
	}
	
	THM3070_SetFWT(0x64);																	//��ʱʱ��Ϊ100*330us=33ms
	THM3070_SendFrame_M(M1_RecvData,0x04);								//��������,����ֵ����ν
	len=0;
	THM3070_RecvFrame_M(M1_RecvData,&len);								//����
	if(len>0)																							//Ӧ��û����Ӧ
	{
		return 1;
	}
	
	return 0;
}


