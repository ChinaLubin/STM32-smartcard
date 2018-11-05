
#include "ISO14443.h"


/*
�ļ���;:			ISO14443Э��
����:				�Ŷ���
����ʱ��:			2018/04/20
����ʱ��:			2018/10/31
�汾:				V1.2

��ʷ�汾:			V1.0:����THM3070ʵ��ISO14443Э��
					V1.1:�޸��˿�ŵ��µ�BUG,�޸��˼�⿨TESTA/B���������쳣������
					V1.2:ȥ��ĳЩ������ʹ�þֲ�����ȥ���ղ��������ݵĲ���

*/



#include "THM3070.h"
#include "string.h"


static const uint16_t TAB_MaximumFarmeSize[16] =  								//���֡���ȱ�
{
    16, 24, 32, 40, 48, 64, 96, 128, 256, 256, 256, 256, 256, 256, 256, 256
};


uint8_t ISO_PCB = 0x0A;															//ͨ�����ݿ�ĵ�1�ֽ�,0x02/0x0A,Ĭ��ΪI��,������,CID����
uint8_t ISO_PICC_CIDSUP = 0x01;													//���Ƿ�֧��CID,Ĭ��֧��
uint32_t ISO_PICC_FWT = 0x64;													//ͨ�ŵȴ���ʱʱ��,Ĭ��100*330us=33ms
uint16_t ISO_PICC_MFSIZE = 16;													//���ܽ��յ����֡��,Ĭ��16


uint8_t ISO_ATQB[16] = {0x00};													//ATQB����
#define ISO_ATQB_PUPI				(ISO_ATQB+1)								//ATQB�е�PUPI
#define ISO_ATQB_APPDATA			(ISO_ATQB+5)								//ATQB�е�Application Data
#define ISO_ATQB_PROTOCOLINFO		(ISO_ATQB+9)								//ATQB�еĲ�����Ϣ
#define ISO_ATQB_PLINFO_SFGI		((ISO_ATQB_PROTOCOLINFO[3]&0xF0)>>4)		//SFGI,����ӦATTRIB�������Ӧ����ʱ��ʱ��
#define ISO_ATQB_PLINFO_FO			((ISO_ATQB_PROTOCOLINFO[2]&0x03)>>0)		//F0����
#define ISO_ATQB_PLINFO_FO_CIDEN	(ISO_ATQB_PLINFO_FO&0x01)					//�Ƿ�֧��CID
#define ISO_ATQB_PLINFO_ADC			((ISO_ATQB_PROTOCOLINFO[2]&0x0C)>>2)		//
#define ISO_ATQB_PLINFO_FWI			((ISO_ATQB_PROTOCOLINFO[2]&0xF0)>>4)		//ͨ�ų�ʱʱ��
#define ISO_ATQB_PLINFO_PTYPE		((ISO_ATQB_PROTOCOLINFO[1]&0x0F)>>0)		//�Ƿ�֧��14443Э��,TR2��Сֵ
#define ISO_ATQB_PLINFO_MFSIZE		((ISO_ATQB_PROTOCOLINFO[1]&0xF0)>>4)		//���֡����(Ҫ���)
#define ISO_ATQB_PLINFO_BAUD		ISO_ATQB_PROTOCOLINFO[0]					//ͨ������

uint8_t ISO_UID[10] = {0x00};													//UID����
uint8_t ISO_ATS[64] = {0x00};													//ATS����
#define ISO_ATS_TL					(ISO_ATS[0])								//ATS����TL
#define ISO_ATS_T0					(ISO_ATS[1])								//ATS��ʽT0
#define ISO_ATS_TCEN				((ISO_ATS_T0>>6)&0x01)						//ATS���Ƿ����TC
#define ISO_ATS_TBEN				((ISO_ATS_T0>>5)&0x01)						//ATS���Ƿ����TB
#define ISO_ATS_TAEN				((ISO_ATS_T0>>4)&0x01)						//ATS���Ƿ����TA
#define ISO_ATS_FSCI				((ISO_ATS_T0>>0)&0x0F)						//���������֡��(Ҫ���)
#define ISO_ATS_TA					(ISO_ATS[2])								//TA����
#define ISO_ATS_TB					(ISO_ATS[2+ISO_ATS_TAEN])					//TB����
#define ISO_ATS_TB_FWI				((ISO_ATS_TB&0xF0)>>4)						//ͨ�ų�ʱʱ��
#define ISO_ATS_TB_SFGI				((ISO_ATS_TB&0x0F)>>0)						//SFGI,����ӦATS�������Ӧ����ʱ��ʱ��
#define ISO_ATS_TC					(ISO_ATS[2+ISO_ATS_TAEN+ISO_ATS_TBEN])		//TC����
#define ISO_ATS_TC_CIDEN			((ISO_ATS_TC&0x02)>>1)						//�Ƿ�֧��CID
#define ISO_ATS_TK					(ISO_ATS+2+ISO_ATS_TAEN+ISO_ATS_TBEN+ISO_ATS_TCEN)		//TK��ʷ�ֽ�
#define ISO_ATS_TKLen 				(ISO_ATS_TL-2-ISO_ATS_TCEN-ISO_ATS_TBEN-ISO_ATS_TAEN)	//TK��ʷ�ֽڳ���



uint8_t ISO_SDataTemp[260];														//���ͻ���ջ���



/*
���ܣ�	TYPEBѰ��
����1��	���͵�ʱ�����=2^slotNum
����2��	�����Ӧ��ATQB����,��֤��ռ�>=12
����3��	�����Ӧ��ATQB���ݳ���
���أ�	ִ�н��
*/
uint8_t REQB(uint8_t slotNum, uint8_t *DAT_ATQB, uint16_t *LEN_ATQB)
{
    uint8_t RSTST;

    uint8_t CMD[3] = {0x05, 0x00, 0x00};										//REQB����
    CMD[2] |= (slotNum & 0x07);													//����ʱ�����

    ISO_PICC_FWT = 0x05;
    THM3070_SetFWT(ISO_PICC_FWT);												//��ʱʱ��Ϊ5*330us=1.65ms
    THM3070_SetTYPEB();															//TYPEBģʽ

    THM3070_SendFrame(CMD, 3);
    RSTST = THM3070_RecvFrame(DAT_ATQB, LEN_ATQB);

    memcpy(ISO_ATQB, DAT_ATQB, *LEN_ATQB);

    return RSTST;
}

/*
���ܣ�	TYPEB����
����1��	���͵�ʱ�����=2^slotNum
����2��	�����Ӧ��ATQB����,��֤��ռ�>=12
����3��	�����Ӧ��ATQB���ݳ���
���أ�	ִ�н��
*/
uint8_t WUPB(uint8_t slotNum, uint8_t *DAT_ATQB, uint16_t *LEN_ATQB)
{
    uint8_t RSTST;

    uint8_t CMD[3] = {0x05, 0x00, 0x08};										//WUPB����
    CMD[2] |= (slotNum & 0x07);													//����ʱ�����

    ISO_PICC_FWT = 0x05;
    THM3070_SetFWT(ISO_PICC_FWT);												//��ʱʱ��Ϊ5*330us=1.65ms
    THM3070_SetTYPEB();															//TYPEBģʽ


    THM3070_SendFrame(CMD, 3);
    RSTST = THM3070_RecvFrame(DAT_ATQB, LEN_ATQB);

    memcpy(ISO_ATQB, DAT_ATQB, *LEN_ATQB);										//����ATQB

    return RSTST;
}

/*
���ܣ�	TYPEB����ʱ���
����1��	���͵�ʱ��������:2-16
����2��	�����Ӧ��ATQB����,��֤��ռ�>=12
����3��	�����Ӧ��ATQB���ݳ���
���أ�	ִ�н��
*/
uint8_t SlotMARKER(uint8_t slotIndex, uint8_t *DAT_ATQB, uint16_t *LEN_ATQB)
{
    uint8_t RSTST;

    uint8_t CMD[1] = {0x05};													//SlotMARKER����
    slotIndex <<= 4;
    slotIndex -= 1;
    CMD[0] |= (slotIndex & 0xF0);												//����ʱ��۱��

    ISO_PICC_FWT = 0x05;
    THM3070_SetFWT(ISO_PICC_FWT);												//��ʱʱ��Ϊ5*330us=1.65ms

    THM3070_SendFrame(CMD, 3);
    RSTST = THM3070_RecvFrame(DAT_ATQB, LEN_ATQB);

    memcpy(ISO_ATQB, DAT_ATQB, *LEN_ATQB);										//����ATQB

    return RSTST;
}

/*
���ܣ�	TYPEB����
����1��	���ATTRIB����Ӧ,��֤��ռ�>=1+n
����2��	���ATTRIB����Ӧ����
���أ�	ִ�н��
*/
uint8_t ATTRIB(uint8_t *DAT_ATTRIBAnswer, uint16_t *LEN_ATTRIBAnswer)
{
    uint8_t RSTST;
    uint8_t TxBuad = 0, RxBuad = 0;

    uint8_t CMD[9] = {0x1D};													//ATTRIB����
    memcpy(CMD + 1, ISO_ATQB_PUPI, 4);											//PUPI
    CMD[5] = 0x00;																//TR0,TR1,EOF,SOF
    CMD[6] = 0x08;																//PCD�������֡256
    /*��������
    if(ISO_ATQB_PLINFO_BAUD!=0x00)												//�ɵ�����
    {
    	uint8_t temp=0x00;

    	if((ISO_ATQB_PLINFO_BAUD&0x40)==0x40)									//PICC TO PCD 848kbit/s
    	{
    		temp|=0xC0;
    		RxBuad=0x03;
    	}
    	else if((ISO_ATQB_PLINFO_BAUD&0x20)==0x20)								//PICC TO PCD 424kbit/s
    	{
    		temp|=0x80;
    		RxBuad=0x02;
    	}
    	else if((ISO_ATQB_PLINFO_BAUD&0x10)==0x10)								//PICC TO PCD 212kbit/s
    	{
    		temp|=0x40;
    		RxBuad=0x01;
    	}
    	CMD[6]|=temp;															//����PICC TO PCD����

    	if((ISO_ATQB_PLINFO_BAUD&0x80)==0x80)									//���ͽ���������ͬ
    	{
    		CMD[6]|=temp>>2;
    	}
    	else
    	{
    		temp=0x00;
    		if((ISO_ATQB_PLINFO_BAUD&0x04)==0x04)								//PCD TO PICC 848kbit/s
    		{
    			temp|=0x30;
    			TxBuad=0x03;
    		}
    		else if((ISO_ATQB_PLINFO_BAUD&0x02)==0x02)							//PCD TO PICC 424kbit/s
    		{
    			temp|=0x20;
    			TxBuad=0x02;
    		}
    		else if((ISO_ATQB_PLINFO_BAUD&0x01)==0x01)							//PCD TO PICC 212kbit/s
    		{
    			temp|=0x10;
    			TxBuad=0x01;
    		}
    		CMD[6]|=temp;														//����PCD TO PICC����
    	}
    }
    ��������*/
    CMD[7] = 0x01;																//֧��14443,TR2
    CMD[8] = 0x00;																//Ϊ�������CID,Ĭ��Ϊ0


    ISO_PICC_FWT = 0x10;
    THM3070_SetFWT(ISO_PICC_FWT);												//��ʱʱ��Ϊ16*330us=5ms

    THM3070_SendFrame(CMD, 9);
    RSTST = THM3070_RecvFrame(DAT_ATTRIBAnswer, LEN_ATTRIBAnswer);

    if(RSTST == THM_RSTST_FEND)
    {
        ISO_PICC_CIDSUP = ISO_ATQB_PLINFO_FO_CIDEN;								//�����Ƿ�֧��CID
        ISO_PICC_FWT = 0x0001 << ISO_ATQB_PLINFO_FWI;							//����ͨ�ų�ʱʱ��
        if(ISO_PICC_FWT < 0x10)
        {
            ISO_PICC_FWT = 0x10;
        }
        if(ISO_PICC_FWT > 0x4000)
        {
            ISO_PICC_FWT = 0x4000;
        }
        ISO_PICC_MFSIZE = TAB_MaximumFarmeSize[ISO_ATQB_PLINFO_MFSIZE]; 		//�������֡����
        if(TxBuad > 0)
        {
            THM3070_SetTxBaud(TxBuad);
        }
        if(RxBuad > 0)
        {
            THM3070_SetRxBaud(RxBuad);
        }

        delay_us(0x0001 << ISO_ATQB_PLINFO_SFGI);

    }

    if(ISO_PICC_CIDSUP)
    {
        ISO_PCB = 0x0A;															//��ʼPCB,֧��CID,PCB=0x0B
    }
    else
    {
        ISO_PCB = 0x02;															//��ʼPCB,��֧��CID,PCB=0x03
    }

    return RSTST;

}

/*
���ܣ�	��λ��+TYPEB����+����
����1��	�����Ӧ��ATQB����,��֤��ռ�>=12
����2��	�����Ӧ��ATQB���ݳ���
���أ�	ִ�н��
*/
uint8_t FINDB(uint8_t *DAT_ATQB, uint16_t *LEN_ATQB)
{
    uint8_t RSTST = THM_RSTST_CERR, slotNum = 0;

    THM3070_RFReset();
    while(RSTST == THM_RSTST_CERR && slotNum < 4)
    {
        RSTST = WUPB(slotNum, DAT_ATQB, LEN_ATQB);
        if(RSTST == THM_RSTST_TMROVER)
        {
            uint8_t temp = 0;

            while(temp + 2 <= (0x01 << slotNum))
            {
                RSTST = SlotMARKER(temp + 2, DAT_ATQB, LEN_ATQB);
                if(RSTST == THM_RSTST_FEND)
                {
                    break;
                }
                temp++;
            }
        }

        slotNum++;
    }
    if(RSTST == THM_RSTST_FEND)
    {
        uint16_t len;

        RSTST = ATTRIB(ISO_SDataTemp, &len);
    }

    return RSTST;
}



/*
���ܣ�	TYPEAѰ��
����1��	�����Ӧ��ATQA����,��֤��ռ�>=2
����2��	�����Ӧ��ATQA���ݳ���
���أ�	ִ�н��
*/
uint8_t REQA(uint8_t *DAT_ATQA, uint16_t *LEN_ATQA)
{
    uint8_t RSTST;

    uint8_t CMD[1] = {0x26};													//WUPA����

    ISO_PICC_FWT = 0x05;
    THM3070_SetFWT(ISO_PICC_FWT);												//��ʱʱ��Ϊ5*330us=1.65ms
    THM3070_SetTYPEA();															//TYPEAģʽ


    THM3070_SendFrame(CMD, 1);													//Э��Ҫ��֡,�������;�ȻҲ����?
    RSTST = THM3070_RecvFrame(DAT_ATQA, LEN_ATQA);

    return RSTST;
}

/*
���ܣ�	TYPEA����
����1��	�����Ӧ��ATQA����,��֤��ռ�>=2
����2��	�����Ӧ��ATQA���ݳ���
���أ�	ִ�н��
*/
uint8_t WUPA(uint8_t *DAT_ATQA, uint16_t *LEN_ATQA)
{
    uint8_t RSTST;

    uint8_t CMD[1] = {0x52};													//WUPA����

    ISO_PICC_FWT = 0x05;
    THM3070_SetFWT(ISO_PICC_FWT);												//��ʱʱ��Ϊ5*330us=1.65ms
    THM3070_SetTYPEA();															//TYPEAģʽ


    THM3070_SendFrame(CMD, 1);													//Э��Ҫ��֡,�������;�ȻҲ����?
    RSTST = THM3070_RecvFrame(DAT_ATQA, LEN_ATQA);

    return RSTST;
}

/*
���ܣ�	���ͷ���ͻָ����,�ڲ�����
����1��	����ͻ����
����2��	���ص�ѡ��ָ������
����3��	���ص�ѡ��ָ�����ݳ���
���أ�	ִ�н��
*/
static uint8_t SendAC(uint8_t casLevel, uint8_t *selCode, uint16_t *Len_selCode)
{
    uint8_t *temp = ISO_SDataTemp + 128;
    uint8_t curReceivePostion, lastPostion, RSTST;
    uint16_t len = 0;

    temp[0] = casLevel;															//SEL
    temp[1] = 0x20;																//NVB=0x20,û����֪��UID
    curReceivePostion = lastPostion = 0x00;

    while(1)
    {
        THM3070_SendFrame(temp, curReceivePostion + 2);							//Э��Ҫ��֡,�������;�ȻҲ����?
        RSTST = THM3070_RecvFrame(temp + lastPostion + 2, &len);
        if(len > 5)
        {
            len = 5;															//�����³���
        }

        curReceivePostion = lastPostion + len;									//�ܹ����յ������ݳ���
        if(len != 0)
        {
            lastPostion += len - 1;												//ȥ�����1�ֽ�,��Ϊ�����ܴ��г�ͻ
        }

        if(RSTST & THM_RSTST_CERR)												//�г�ͻ
        {
            delay_ms(6);														//��Ҫ��ʱ�ѳ�ͻ֮����ֽڹ��˵�
            temp[1] = THM3070_ReadREG(THM_REG_BITPOS) + 1;						//���յ��ı���λ����,NVB��4λ
            temp[1] += (uint8_t)(len + 1) << 4;									//���յ����ֽڳ���,NVB��4λ
            if((temp[1] & 0x0f) == 0x08)										//����λ����Ϊ8
            {
                temp[1] = ((temp[1] & 0xf0) + 0x10);							//����λ����,�ֽ�+1
                lastPostion = (lastPostion + 1);								//+1
            }
        }
        else if(RSTST == THM_RSTST_FEND || RSTST == THM_RSTST_CRCERR)
        {
            if(lastPostion == 4)
            {
                memcpy(selCode + 2, temp + 2, 5);								//û�г�ͻ,����ѡ��ָ������
                *Len_selCode = 7;												//����Ϊ7

                return THM_RSTST_FEND;
            }
            else
            {
                return THM_RSTST_OTHER;
            }
        }
        else
        {
            return RSTST;														//����
        }
    }
}

/*
���ܣ�	TYPEA����ͻ+ѡ��
����1��	��UID,��֤��ռ�>=10
����2��	��UID����
���أ�	ִ�н��
*/
uint8_t AnticollAndSelect(uint8_t *DAT_UID, uint16_t *LEN_UID)
{
    uint8_t *UIDTemp = ISO_SDataTemp;											//�ݴ��յ���UID,���ܰ��������ַ�CT=0x88

    uint8_t RSTST, CASLEVEL = 0x93;
    uint8_t *selCode = ISO_SDataTemp + 64;
    uint16_t len;
    uint8_t i, count;

    *LEN_UID = 0x00;
    ISO_PICC_FWT = 0x10;
    THM3070_SetFWT(ISO_PICC_FWT);												//��ʱʱ��Ϊ16*330us=5.28ms

    for(i = 0; i < 3; i++)
    {
        count = 3;
        while(count--)
        {
            RSTST = SendAC(CASLEVEL, selCode, &len);							//����SEL=0x93/0x95/0x97
            if(RSTST == THM_RSTST_FEND)
            {
                break;
            }
        }
        if(RSTST == THM_RSTST_FEND)
        {
            memcpy(UIDTemp + i * 5, selCode + 2, 5);							//��ȡ��UID,���ܰ���CT
        }
        else
        {
            return RSTST;
        }

        count = 3;
        while(count--)
        {
            selCode[0] = CASLEVEL;
            selCode[1] = 0x70;													//ѡ��
            THM3070_SendFrame(selCode, 7);										//
            selCode[0] = 0;
            RSTST = THM3070_RecvFrame(selCode, &len);							//��ӦSAK
            if(RSTST == THM_RSTST_FEND)
            {
                break;
            }
        }
        if(RSTST == THM_RSTST_FEND)
        {
            if((selCode[0] & 0x04) != 0x00)										//SAK��3λΪ1����UID������
            {
                CASLEVEL += 2;													//������һ��
                memcpy(DAT_UID + i * 3, UIDTemp + i * 5 + 1, 3);				//��ȡ������UID
            }
            else
            {
                memcpy(DAT_UID + i * 3, UIDTemp + i * 5, 4);					//UID����,��ȡ��UID
                break;
            }
        }
        else
        {
            return RSTST;
        }
    }
    *LEN_UID = 4 + i * 3;														//UID����4/7/10
    memcpy(ISO_UID, DAT_UID, *LEN_UID);											//����UID

    return RSTST;
}

/*
���ܣ�	TYPEA����
����1��	�����ص�ATS,��֤��ռ�>=5+n
����2��	ATS�ĳ���
���أ�	ִ�н��
*/
uint8_t RATS(uint8_t *DAT_ATS, uint16_t *LEN_ATS)
{
    uint8_t RSTST;

    uint8_t CMD[2] = {0xE0, 0x80};												//RATS����,���֡��256,CID=0

    ISO_PICC_FWT = 0x10;
    THM3070_SetFWT(ISO_PICC_FWT);												//��ʱʱ��Ϊ16*330us=5ms


    THM3070_SendFrame(CMD, 2);													//
    RSTST = THM3070_RecvFrame(DAT_ATS, LEN_ATS);

    if(RSTST == THM_RSTST_FEND)
    {
        memcpy(ISO_ATS, DAT_ATS, *LEN_ATS);										//����ATS

        ISO_PICC_MFSIZE = TAB_MaximumFarmeSize[ISO_ATS_FSCI]; 					//�������֡����
        if(ISO_ATS_TAEN)
        {
            ;
        }
        if(ISO_ATS_TBEN)
        {
            ISO_PICC_FWT = 0x0001 << ISO_ATS_TB_FWI;							//����ͨ�ų�ʱʱ��
            if(ISO_PICC_FWT < 0x10)
            {
                ISO_PICC_FWT = 0x10;
            }
            if(ISO_PICC_FWT > 0x4000)
            {
                ISO_PICC_FWT = 0x4000;
            }
            delay_us(0x0001 << ISO_ATS_TB_SFGI);								//��ʱ
        }
        if(ISO_ATS_TCEN)
        {
            ISO_PICC_CIDSUP = ISO_ATS_TC_CIDEN;									//�����Ƿ�֧��CID
        }

        if(ISO_PICC_CIDSUP)
        {
            ISO_PCB = 0x0A;														//��ʼPCB,֧��CID,PCB=0x0B
        }
        else
        {
            ISO_PCB = 0x02;														//��ʼPCB,��֧��CID,PCB=0x03
        }

    }

    return RSTST;
}

/*
���ܣ�	TYPEAPPS
����1��	��������(��������������,0x00:106kbit/s,0x01:212,0x02:424,0x03:848kbit/s)
����2��	��������(��������������,0x00:106kbit/s,0x01:212,0x02:424,0x03:848kbit/s)
���أ�	ִ�н��
*/
uint8_t PPSS(uint8_t TxBuad, uint8_t RxBuad)
{
    uint8_t RSTST;
    uint16_t len;

    uint8_t CMD[3] = {0xD0, 0x11, 0x00};										//PPSS����,CID=0
    CMD[2] |= (TxBuad & 0x03);
    CMD[2] |= ((RxBuad & 0x03) << 2);

    ISO_PICC_FWT = 0x05;
    THM3070_SetFWT(ISO_PICC_FWT);												//��ʱʱ��Ϊ5*330us=1.65ms

    THM3070_SendFrame(CMD, 3);													//
    RSTST = THM3070_RecvFrame(ISO_SDataTemp, &len);

    if(RSTST == THM_RSTST_FEND)
    {
        THM3070_SetTxBaud(TxBuad);
        THM3070_SetRxBaud(RxBuad);
    }

    return RSTST;
}

/*
���ܣ�	��λ��+TYPEA����+����ͻ+ѡ��+����
����1��	�����ص�ATS,��֤��ռ�>=5+n
����2��	ATS�ĳ���
���أ�	ִ�н��
*/
uint8_t FINDA(uint8_t *DAT_ATS, uint16_t *LEN_ATS)
{
    uint8_t RSTST;
    uint16_t len;

    THM3070_RFReset();
    RSTST = WUPA(ISO_SDataTemp, &len);
    if(RSTST == THM_RSTST_FEND || RSTST == THM_RSTST_CERR)
    {
        RSTST = AnticollAndSelect(ISO_SDataTemp, &len);
        if(RSTST == THM_RSTST_FEND)
        {
            RSTST = RATS(ISO_SDataTemp, &len);
            *LEN_ATS = len;
            memcpy(DAT_ATS, ISO_SDataTemp, len);
        }
    }
    return RSTST;
}



/*
���ܣ�	����ԭʼ���ݲ���ȡ��Ӧ
����1��	�����͵�����
����2��	�����͵����ݳ���
����3��	���յ�������,��֤��ռ��㹻
����4��	���յ������ݳ���
���أ�	ִ�н��
*/
uint8_t ExchangeData(uint8_t *sData, uint16_t len_sData, uint8_t *rData, uint16_t *len_rData)
{
    uint8_t RSTST;

    THM3070_SetFWT(ISO_PICC_FWT);

    THM3070_SendFrame(sData, len_sData);
    RSTST = THM3070_RecvFrame(rData, len_rData);

    return RSTST;
}


/*
���ܣ�	���췢�Ϳ�,�ڲ�����
����1��	����õĿ�,��֤��ռ�>=len+2
����2��	�����������
����3��	����������ݼ�����ú�Ŀ鳤��
���أ�	��
*/
static void formBlock(uint8_t *block, const uint8_t *sData, uint16_t *len)
{
    uint16_t i;

    if(ISO_PICC_CIDSUP)
    {
        for(i = *len + 2; i > 1; i--)											//ѭ����λ
        {
            block[i] = sData[i - 2];											//�������ǰ,��ֹͬ��ַ����
        }
        block[0] = 0x00;														//PCD��0
        block[1] = 0x00;														//CIDΪ0
        *len += 2;																//����+PCB+CID
    }
    else
    {
        for(i = *len + 1; i > 0; i--)
        {
            block[i] = sData[i - 1];
        }
        block[0] = 0x00;														//PCD��0
        *len += 1;																//����+PCB
    }
}

/*
���ܣ�	����APDU����,�����ط�һ��
����1��	Ҫ���͵�APDU����
����2��	Ҫ���͵����ݳ���
����3��	���յ�������,��֤��ռ��㹻
����4��	���յ������ݳ���
���أ�	ִ�н��
*/
uint8_t ExchangeAPDU(uint8_t *sData, uint16_t len_sData, uint8_t *rData, uint16_t *len_rData)
{
    uint8_t e1, e2, e3;															//����

    uint8_t RSTST = THM_RSTST_TMROVER;											//����״̬
    uint16_t SendLen1, SendLen2, RecvLen = 0, Temp, LenTemp;

    SendLen1 = len_sData;														//�������ݳ���
    SendLen2 = SendLen1;
    while(SendLen1)																//�жϳ���ѭ������
    {
        e1 = e2 = e3 = 0;

        if(ISO_PICC_CIDSUP)														//CID����
        {
            Temp = ISO_PICC_MFSIZE - 4;											//���֡��-PCD-CID-2CRC
        }
        else
        {
            Temp = ISO_PICC_MFSIZE - 3;											//���֡��-PCD-2CRC
        }
        if(SendLen1 > Temp)														//����>���֡��
        {
            SendLen1 -= Temp;													//����-
            formBlock(ISO_SDataTemp, sData, &Temp);								//����������λ,����1�ֽ�PCB�ó���

            ISO_SDataTemp[0] = ISO_PCB;											//��ֵPCB
            ISO_SDataTemp[0] |= 0x10;											//��������,��Ϊ���ݳ�һ�η��Ͳ���,�������ӷ���

        }
        else
        {
            Temp = SendLen1;													//��¼����
            SendLen1 = 0;														//����Ϊ0
            formBlock(ISO_SDataTemp, sData, &Temp);

            ISO_SDataTemp[0] = ISO_PCB;											//��ֵPCB
            ISO_SDataTemp[0] &= 0xEF;											//������
        }
        RSTST = ExchangeData(ISO_SDataTemp, Temp, rData + RecvLen, len_rData);	//����
sok:
        if(RSTST == THM_RSTST_FEND)												//�ɹ�
        {
            uint8_t chaining = 0;												//�����Ƿ�����,Ĭ�ϲ�����
            while(1)
            {
                if(RSTST == THM_RSTST_FEND)										//�ɹ�
                {
                    chaining = 0;												//������

                    Temp = RecvLen;												//�ݴ�һ��,���յ�������λȥ��PCB��ʱ��Ҫ��
                    ISO_SDataTemp[0] = rData[RecvLen];							//��ȡ���յ���PCB

                    if((ISO_SDataTemp[0] | 0x3F) == 0x3F)						//ΪI��,��2λΪ0
                    {
                        if(ISO_SDataTemp[0] & 0x01)
                        {
                            ISO_PCB &= 0xFE;									//�յ�I��,��ź��յ����෴
                        }
                        else
                        {
                            ISO_PCB |= 0x01;
                        }
                        if((ISO_SDataTemp[0] & 0x10) == 0x10)					//����,����λΪ1
                        {
                            chaining = 1;										//����
                        }
                    }
                    else if((ISO_SDataTemp[0] & 0xC0) == 0xC0)					//ΪS��,��2λΪ1
                    {
                        while((RSTST == THM_RSTST_FEND) && (ISO_SDataTemp[0] & 0xC0) == 0xC0)	//�ɹ��ҷ�����ΪS��
                        {
                            uint8_t swtx[4] = {0xFA, 0x00, 0x00};
                            uint32_t ISO_PICC_FWT_Copy;

                            ISO_PICC_FWT_Copy = ISO_PICC_FWT;

                            if(ISO_PICC_CIDSUP)									//CIDʹ��
                            {
                                swtx[0] = 0xFA;									//CID����ʱ��S��PCB
                                swtx[1] = rData[RecvLen + 1];					//CIDʹ�÷��������е�CID
                                swtx[2] = rData[RecvLen + 2] & 0x3F;			//���췵��S(SWT)����
                                swtx[3] = 0x03;									//���ݳ���

                                ISO_PICC_FWT = ISO_PICC_FWT * swtx[2];			//����ȴ�ʱ��
                                if(ISO_PICC_FWT == 0)
                                {
                                    ISO_PICC_FWT = ISO_PICC_FWT_Copy;
                                }
                                if(ISO_PICC_FWT > 0x4000)						//����
                                {
                                    ISO_PICC_FWT = 0x4000;
                                }
                            }
                            else
                            {
                                swtx[0] = 0xF2;									//CID������ʱ��S��PCB
                                swtx[1] = rData[RecvLen + 1] & 0x3F;
                                swtx[3] = 0x02;

                                ISO_PICC_FWT = ISO_PICC_FWT * swtx[2];			//����ȴ�ʱ��
                                if(ISO_PICC_FWT == 0)
                                {
                                    ISO_PICC_FWT = ISO_PICC_FWT_Copy;
                                }
                                if(ISO_PICC_FWT > 0x4000)						//����
                                {
                                    ISO_PICC_FWT = 0x4000;
                                }
                            }

                            RSTST = ExchangeData(swtx, swtx[3], rData + RecvLen, len_rData);	//����
                            ISO_PICC_FWT = ISO_PICC_FWT_Copy;					//�ȴ�ʱ��ָ�
                            ISO_SDataTemp[0] = rData[RecvLen];

                            if(RSTST != THM_RSTST_FEND && e2 == 0)				//���ɹ�
                            {
                                e2 = 1;

                                LenTemp = 0;
                                formBlock(swtx, swtx, &LenTemp);				//�����
                                swtx[0] = ISO_PCB;
                                if(ISO_PICC_CIDSUP)								//CIDʹ��
                                {
                                    swtx[0] |= 0xB8;
                                    swtx[0] &= 0xBF;							//����R��(NAK),CIDλ��1
                                }
                                else
                                {
                                    swtx[0] |= 0xB0;
                                    swtx[0] &= 0xB7;							//����R��(NAK),CIDλ��0
                                }

                                RSTST = ExchangeData(swtx, LenTemp, rData + RecvLen, len_rData);	//����
                                ISO_SDataTemp[0] = rData[RecvLen];
                            }
                        }
                        continue;
                    }
                    else														//ΪR��
                    {
                        if(ISO_SDataTemp[0] & 0x01)
                        {
                            ISO_PCB &= 0xFE;									//�յ�I��,��ź��յ����෴
                        }
                        else
                        {
                            ISO_PCB |= 0x01;
                        }
                        if(e1 == 1)												//����ϴ��д���,�յ�R���ʾ���������·���֮ǰ���͵�����
                        {
                            SendLen1 = SendLen2;								//�ָ�����λ�����·���
                        }
                        break;													//�ϴ��޴���,������������
                    }
                    RecvLen = RecvLen + *len_rData;
                    if(ISO_PICC_CIDSUP)
                    {
                        RecvLen -= 2;
                        for(; Temp < RecvLen; Temp++)
                        {
                            rData[Temp] = rData[Temp + 2];						//��λȥ�����ͷ
                        }
                    }
                    else
                    {
                        RecvLen -= 1;
                        for(; Temp < RecvLen; Temp++)
                        {
                            rData[Temp] = rData[Temp + 1];
                        }
                    }
                    SendLen2 = SendLen1;										//��¼����λ��,���ڴ����ط�

                    if(!chaining)break;											//������,��������

                    LenTemp = 0;
                    formBlock(ISO_SDataTemp, ISO_SDataTemp, &LenTemp);
                    ISO_SDataTemp[0] = ISO_PCB;									//������Ҫ����ACK,����R��(ACK)
                    if(ISO_PICC_CIDSUP)											//CIDʹ��
                    {
                        ISO_SDataTemp[0] |= 0xA8;
                        ISO_SDataTemp[0] &= 0xAF;								//����R��(ACK),CIDλ��1
                    }
                    else
                    {
                        ISO_SDataTemp[0] |= 0xA0;
                        ISO_SDataTemp[0] &= 0xA7;								//����R��(ACK),CIDλ��0
                    }

                    RSTST = ExchangeData(ISO_SDataTemp, LenTemp, rData + RecvLen, len_rData);	//����ACK,������ŷ�����һ����������
                }//if(RSTST==THM_RSTST_FEND)
                else
                {
                    if(e3 == 0)
                    {
                        e3 = 1;

                        LenTemp = 0;
                        formBlock(ISO_SDataTemp, ISO_SDataTemp, &LenTemp);
                        ISO_SDataTemp[0] = ISO_PCB;								//����R��(ACK)
                        if(ISO_PICC_CIDSUP)										//CIDʹ��
                        {
                            ISO_SDataTemp[0] |= 0xA8;
                            ISO_SDataTemp[0] &= 0xAF;							//����R��(ACK),CIDλ��1
                        }
                        else
                        {
                            ISO_SDataTemp[0] |= 0xA0;
                            ISO_SDataTemp[0] &= 0xA7;							//����R��(ACK),CIDλ��0
                        }

                        RSTST = ExchangeData(ISO_SDataTemp, LenTemp, rData + RecvLen, len_rData);	//����ACK,�����ٴη����ϴ�����

                        continue;
                    }
                    else
                    {
                        break;													//���ս���
                    }
                }
            }//while(1)
        }//sok:if(RSTST==THM_RSTST_FEND)
        else																	//��һ�η���,����Ӧ������
        {
            if(e1 == 0)															//e�������ƴ���
            {
                e1 = 1;
                LenTemp = 0;
                formBlock(ISO_SDataTemp, ISO_SDataTemp, &LenTemp);
                ISO_SDataTemp[0] = ISO_PCB;										//����R��(NAK)
                if(ISO_PICC_CIDSUP)												//CIDʹ��
                {
                    ISO_SDataTemp[0] |= 0xB8;
                    ISO_SDataTemp[0] &= 0xBF;									//����R��(NAK),CIDλ��1
                }
                else
                {
                    ISO_SDataTemp[0] |= 0xB0;
                    ISO_SDataTemp[0] &= 0xB7;									//����R��(NAK),CIDλ��0
                }

                RSTST = ExchangeData(ISO_SDataTemp, LenTemp, rData + RecvLen, len_rData);	//����NAK,������ӦACK�����ϴλ��͵�����I��,ȡ�����ϴ�˭����

                goto sok;														//�����ж�
            }
            else
            {
                break;															//���ͽ���
            }
        }

    }//while(SendLen1)
    *len_rData = RecvLen;

    return RSTST;
}

/*
���ܣ�	����TYPEA���Ƿ��ڳ���
����1��	��
���أ�	ִ�н��
*/
uint8_t TESTA()
{
    uint8_t RSTST = 0, swtx[4];
    uint16_t len;

    len = 0;
    formBlock(swtx, swtx, &len);												//�����

    swtx[0] = ISO_PCB;															//ʹ��2-a�ļ�ⷽʽ R(NAK)1 <-> R(ACK)0
    if(ISO_PICC_CIDSUP)															//CIDʹ��
    {
        swtx[0] |= 0xB8;
        swtx[0] &= 0xBF;														//����R��(NAK),CIDλ��1
    }
    else
    {
        swtx[0] |= 0xB0;
        swtx[0] &= 0xB7;														//����R��(NAK),CIDλ��0
    }

    RSTST = ExchangeData(swtx, len, ISO_SDataTemp, &len);						//����


    return RSTST;
}

/*
���ܣ�	����TYPEB���Ƿ��ڳ���
����1��	��
���أ�	ִ�н��
*/
uint8_t TESTB()
{
    uint8_t RSTST = TESTA();

    return RSTST;
}




