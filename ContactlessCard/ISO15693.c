
#include "ISO15693.h"


/*
�ļ���;:           ISO15693Э��
����:               �Ŷ���
����ʱ��:           2018/10/22
����ʱ��:           2018/10/22
�汾:               V1.0

��ʷ�汾:           V1.0:����THM3070ʵ��ISO45693Э��


*/


static uint8_t ISO_SelectFlag = 0;                                              //��Ƭ�Ƿ���ѡ��̬
static uint8_t ISO_UIDTemp[10];                                                 //�ݴ濨ƬUID
static uint8_t ISO_SendTemp[32];                                                //���ͽ������ݻ���

#include "string.h"


/*
���ܣ�  ����ͻ
����1�� ��֪��UID����
����2�� ��֪��UIDλ��
����3�� ����UID�Ŀռ�
����4�� ����AFI(��8)|AFI(��8)
���أ�  ִ�н��
*/
static uint8_t collision(uint8_t *maskData, uint8_t maskLen, uint8_t *DAT_UID, uint16_t AFIEN)
{
    uint8_t RSTST, select;
    uint16_t len;

    uint8_t maskDataT[8], maskLenT;

    if(maskLen > 64)                                                            //���64λ
    {
        return THM_RSTST_TMROVER;
    }

    maskLenT = maskLen;                                                         //�ݴ�maskλ����

    if(AFIEN > 0x00FF)
    {
        ISO_SendTemp[0] = 0x36;
        ISO_SendTemp[1] = 0x01;
        ISO_SendTemp[2] = AFIEN & 0x00FF;
        ISO_SendTemp[3] = maskLen;
        len = maskLen / 8;
        if(maskLen % 8 > 0)
        {
            len += 1;                                                           //�����ֽڳ���
        }
        memcpy(ISO_SendTemp + 4, maskData, len);
        if(len > 0)
        {
            memcpy(maskDataT, maskData, len);                                   //�ݴ�mask����
        }
        len += 4;

    }
    else
    {
        ISO_SendTemp[0] = 0x26;
        ISO_SendTemp[1] = 0x01;
        ISO_SendTemp[2] = maskLen;
        len = maskLen / 8;
        if(maskLen % 8 > 0)
        {
            len += 1;                                                           //�����ֽڳ���
        }
        memcpy(ISO_SendTemp + 3, maskData, len);
        if(len > 0)
        {
            memcpy(maskDataT, maskData, len);                                   //�ݴ�mask����
        }
        len += 3;
    }

    THM3070_SendFrame_V(ISO_SendTemp, len);
    RSTST = THM3070_RecvFrame_V(ISO_SendTemp, &len);
    if(RSTST == THM_RSTST_FEND)                                                 //�ɹ�
    {
        memcpy(DAT_UID, ISO_SendTemp + 2, 0x08);
        memcpy(ISO_UIDTemp, ISO_SendTemp + 2, 0x08);
    }
    else if(RSTST == THM_RSTST_CERR)                                            //��ͻ
    {
        delay_ms(6);
        if(len > 10)
        {
            len = 10;                                                           //��һ�³���
        }
        if(len > 2)                                                             //UID��ͻ
        {
            memcpy(maskDataT, ISO_SendTemp + 2, len - 2);                       //������֪��UID
            maskLenT = THM3070_ReadREG(THM_REG_BITPOS) + 1;                     //��ȡ��ͻλ
            select = 0x80 >> (maskLenT - 1);

            //maskDataT[len-3]|=select;                                         //��1��ͻλ,����ѡ��UID
            //RSTST=collision(maskDataT,maskLenT+(len-3)*8,DAT_UID,AFIEN);

            maskDataT[len - 3] &= (~select);                                    //��0��ͻλ,����ѡ��UID
            RSTST = collision(maskDataT, maskLenT + (len - 3) * 8, DAT_UID, AFIEN);  //�ݹ����
        }
        else                                                                    //UIDǰ��ͻ(��ʱUID����֪��,����һλһλ��������֪��UIDλ��(��Ϊ0����Ϊ1))
        {
            select = 0x80;
            maskLenT++;                                                         //maskλ����+1
            len = maskLenT / 8;
            if(maskLenT % 8 > 0)
            {
                len += 1;                                                       //�����ֽڳ���
                select = 0x10 >> ((maskLenT % 8) - 1);
            }

            maskDataT[len - 1] &= (~select);                                    //��֪λ���һλ��0
            RSTST = collision(maskDataT, maskLenT, DAT_UID, AFIEN);             //�ݹ����
            if(RSTST == THM_RSTST_TMROVER && ISO_SelectFlag == 0)               //��һ�γ���û����Ӧ
            {
                ISO_SelectFlag = 1;                                             //���ñ�ʶ,����������ע�ⲻ��֮ǰ����˼��
                maskDataT[len - 1] |= select;                                   //��֪λ���һλ��1(0��1����һ��Ҫ��Ӧ)
                RSTST = collision(maskDataT, maskLenT, DAT_UID, AFIEN);         //�ݹ����
            }
        }
    }

    return RSTST;
}

/*
���ܣ�  ����
����1�� AFIʹ��(��8)|AFI(��8)
����1�� ����UID�Ŀռ�
���أ�  ִ�н��
*/
uint8_t FINDV(uint16_t ENAndAFI, uint8_t *DAT_UID)
{
    uint8_t RSTST;

    THM3070_RFReset();                                                          //��λ��

    RSTST = Inventory(ENAndAFI, DAT_UID);

    return RSTST;
}

/*
���ܣ�  ���
����1�� AFIʹ��(��8)|AFI(��8)
����2�� ����UID�Ŀռ�
���أ�  ִ�н��
*/
uint8_t Inventory(uint16_t ENAndAFI, uint8_t *DAT_UID)
{
    uint8_t RSTST;

    THM3070_SetTYPEV();
    THM3070_SetFWT(0x05);                                                       //��ʱʱ��Ϊ5*330us=1.65ms

    ISO_SelectFlag = 0;                                                         //Ĭ�ϵ�ַģʽ
    RSTST = collision(0x00, 0x00, DAT_UID, ENAndAFI);
    ISO_SelectFlag = 0;                                                         //Ĭ�ϵ�ַģʽ

    return RSTST;
}



/*
���ܣ�	��Ĭ
����1��	��
���أ�	ִ�н��
*/
uint8_t Stayquiet()
{
    ISO_SendTemp[0x00] = 0x22;
    ISO_SendTemp[0x01] = 0x02;
    memcpy(ISO_SendTemp + 2, ISO_UIDTemp, 0x08);                                //UID��ǿ�Ƶ�

    THM3070_SendFrame_V(ISO_SendTemp, 0x0A);

    return THM_RSTST_FEND;
}

/*
���ܣ�  ѡ��
����1�� ��
���أ�  ִ�н��
*/
uint8_t Select()
{
    uint8_t RSTST;
    uint16_t len;

    ISO_SendTemp[0x00] = 0x22;
    ISO_SendTemp[0x01] = 0x25;
    memcpy(ISO_SendTemp + 2, ISO_UIDTemp, 0x08);                                //UID��ǿ�Ƶ�

    THM3070_SendFrame_V(ISO_SendTemp, 0x0A);
    RSTST = THM3070_RecvFrame_V(ISO_SendTemp, &len);
    if(RSTST == THM_RSTST_FEND && ISO_SendTemp[0x00] != 0x00)
    {
        RSTST = ISO_SendTemp[1];
    }
    if(RSTST == THM_RSTST_FEND)
    {
        ISO_SelectFlag = 1;                                                     //����Ϊѡ��ģʽ
    }
    return RSTST;
}

/*
���ܣ�	��λ��׼��
����1��	��
���أ�	ִ�н��
*/
uint8_t ResetToReady()
{
    uint8_t RSTST;
    uint16_t len;

    if(ISO_SelectFlag == 0)                                                     //��ַģʽ,��UID
    {
        ISO_SendTemp[0x00] = 0x22;
        ISO_SendTemp[0x01] = 0x26;
        memcpy(ISO_SendTemp + 2, ISO_UIDTemp, 0x08);

        THM3070_SendFrame_V(ISO_SendTemp, 0x0A);
    }
    else                                                                        //ѡ��ģʽ,����UID
    {
        ISO_SendTemp[0x00] = 0x12;
        ISO_SendTemp[0x01] = 0x26;

        THM3070_SendFrame_V(ISO_SendTemp, 0x02);
    }
    RSTST = THM3070_RecvFrame_V(ISO_SendTemp, &len);
    if(RSTST == THM_RSTST_FEND && ISO_SendTemp[0x00] != 0x00)
    {
        RSTST = ISO_SendTemp[1];
    }

    return RSTST;
}

/*
���ܣ�  ��ȡ��
����1�� ���
����2�� ��ſ�����
����3�� ��ſ����ݵĳ���
���أ�  ִ�н��
*/
uint8_t ReadBlocks(uint8_t BlockNum, uint8_t *BlockData, uint16_t *BlockDataLen)
{
    uint8_t RSTST;
    uint16_t len;

    if(ISO_SelectFlag == 0)                                                     //��ַģʽ,��UID
    {
        ISO_SendTemp[0x00] = 0x22;
        ISO_SendTemp[0x01] = 0x20;
        memcpy(ISO_SendTemp + 2, ISO_UIDTemp, 0x08);
        ISO_SendTemp[0x0A] = BlockNum;

        THM3070_SendFrame_V(ISO_SendTemp, 0x0B);
    }
    else                                                                       //ѡ��ģʽ,����UID
    {
        ISO_SendTemp[0x00] = 0x12;
        ISO_SendTemp[0x01] = 0x20;
        ISO_SendTemp[0x02] = BlockNum;

        THM3070_SendFrame_V(ISO_SendTemp, 0x03);
    }
    RSTST = THM3070_RecvFrame_V(ISO_SendTemp, &len);
    if(RSTST == THM_RSTST_FEND && ISO_SendTemp[0x00] != 0x00)
    {
        RSTST = ISO_SendTemp[1];
    }
    if(RSTST == THM_RSTST_FEND)
    {
        memcpy(BlockData, ISO_SendTemp + 1, len - 1);
        *BlockDataLen = len - 1;
    }
    return RSTST;
}

/*
���ܣ�  д���
����1�� ���
����2�� ������
����3�� �����ݵĳ���
���أ�  ִ�н��
*/
uint8_t WriteBlocks(uint8_t BlockNum, uint8_t *BlockData, uint16_t BlockDataLen)
{
    uint8_t RSTST;
    uint16_t len;

    if(ISO_SelectFlag == 0)	                                                   //��ַģʽ,��UID
    {
        ISO_SendTemp[0x00] = 0x22;
        ISO_SendTemp[0x01] = 0x21;
        memcpy(ISO_SendTemp + 2, ISO_UIDTemp, 0x08);
        ISO_SendTemp[0x0A] = BlockNum;
        memcpy(ISO_SendTemp + 0x0B, BlockData, BlockDataLen);

        THM3070_SendFrame_V(ISO_SendTemp, BlockDataLen + 0x0B);
    }
    else                                                                       //ѡ��ģʽ,����UID
    {
        ISO_SendTemp[0x00] = 0x12;
        ISO_SendTemp[0x01] = 0x21;
        ISO_SendTemp[0x02] = BlockNum;
        memcpy(ISO_SendTemp + 0x03, BlockData, BlockDataLen);

        THM3070_SendFrame_V(ISO_SendTemp, BlockDataLen + 0x03);
    }
    RSTST = THM3070_RecvFrame_V(ISO_SendTemp, &len);

    if(RSTST == THM_RSTST_FEND && ISO_SendTemp[0x00] != 0x00)
    {
        RSTST = ISO_SendTemp[1];
    }
    return RSTST;
}

/*
���ܣ�  ��ȡ�����
����1�� �׸����
����2�� Ҫ��ȡ�Ŀ����(0x00Ϊ����1)
����3�� ��ſ�����
����4�� ��ſ����ݵĳ���
���أ�  ִ�н��
*/
uint8_t ReadMultipleBlocks(uint8_t BlockNum, uint8_t BlockLen, uint8_t *BlockData, uint16_t *BlockDataLen)
{
    uint8_t RSTST;
    uint16_t len;

    if(ISO_SelectFlag == 0)                                                    //��ַģʽ,��UID
    {
        ISO_SendTemp[0x00] = 0x22;
        ISO_SendTemp[0x01] = 0x23;
        memcpy(ISO_SendTemp + 2, ISO_UIDTemp, 0x08);
        ISO_SendTemp[0x0A] = BlockNum;
        ISO_SendTemp[0x0B] = BlockLen;

        THM3070_SendFrame_V(ISO_SendTemp, 0x0C);
    }
    else                                                                       //ѡ��ģʽ,����UID
    {
        ISO_SendTemp[0x00] = 0x12;
        ISO_SendTemp[0x01] = 0x23;
        ISO_SendTemp[0x02] = BlockNum;
        ISO_SendTemp[0x03] = BlockLen;

        THM3070_SendFrame_V(ISO_SendTemp, 0x04);
    }
    RSTST = THM3070_RecvFrame_V(ISO_SendTemp, &len);
    if(RSTST == THM_RSTST_FEND && ISO_SendTemp[0x00] != 0x00)
    {
        RSTST = ISO_SendTemp[1];
    }
    if(RSTST == THM_RSTST_FEND)
    {
        memcpy(BlockData, ISO_SendTemp + 1, len - 1);
        *BlockDataLen = len - 1;
    }
    return RSTST;
}

/*
���ܣ�  ��ȡ��
����1�� �׸����
����2�� Ҫд��Ŀ����(0x00Ϊ����1)
����3�� ������
����4�� �����ݵĳ���
���أ�  ִ�н��
*/
uint8_t WriteMultipleBlocks(uint8_t BlockNum, uint8_t BlockLen, uint8_t *BlockData, uint16_t BlockDataLen)
{
    uint8_t RSTST;
    uint16_t len;

    if(ISO_SelectFlag == 0)                                                    //��ַģʽ,��UID
    {
        ISO_SendTemp[0x00] = 0x22;
        ISO_SendTemp[0x01] = 0x24;
        memcpy(ISO_SendTemp + 2, ISO_UIDTemp, 0x08);
        ISO_SendTemp[0x0A] = BlockNum;
        ISO_SendTemp[0x0B] = BlockLen;
        memcpy(ISO_SendTemp + 0x0C, BlockData, BlockDataLen);

        THM3070_SendFrame_V(ISO_SendTemp, BlockDataLen + 0x0C);
    }
    else                                                                       //ѡ��ģʽ,����UID
    {
        ISO_SendTemp[0x00] = 0x12;
        ISO_SendTemp[0x01] = 0x24;
        ISO_SendTemp[0x02] = BlockNum;
        ISO_SendTemp[0x03] = BlockLen;
        memcpy(ISO_SendTemp + 0x04, BlockData, BlockDataLen);

        THM3070_SendFrame_V(ISO_SendTemp, BlockDataLen + 0x04);
    }
    RSTST = THM3070_RecvFrame_V(ISO_SendTemp, &len);

    if(RSTST == THM_RSTST_FEND && ISO_SendTemp[0x00] != 0x00)
    {
        RSTST = ISO_SendTemp[1];
    }
    return RSTST;
}

/*
���ܣ�  дAFI
����1�� AFI
���أ�  ִ�н��
*/
uint8_t WriteAFI(uint8_t AFI)
{
    uint8_t RSTST;
    uint16_t len;

    if(ISO_SelectFlag == 0)                                                    //��ַģʽ,��UID
    {
        ISO_SendTemp[0x00] = 0x22;
        ISO_SendTemp[0x01] = 0x27;
        memcpy(ISO_SendTemp + 2, ISO_UIDTemp, 0x08);
        ISO_SendTemp[0x0A] = AFI;

        THM3070_SendFrame_V(ISO_SendTemp, 0x0B);
    }
    else                                                                       //ѡ��ģʽ,����UID
    {
        ISO_SendTemp[0x00] = 0x12;
        ISO_SendTemp[0x01] = 0x27;
        ISO_SendTemp[0x02] = AFI;

        THM3070_SendFrame_V(ISO_SendTemp, 0x03);
    }
    RSTST = THM3070_RecvFrame_V(ISO_SendTemp, &len);
    if(RSTST == THM_RSTST_FEND && ISO_SendTemp[0x00] != 0x00)
    {
        RSTST = ISO_SendTemp[1];
    }

    return RSTST;
}

/*
���ܣ�  дDSFID
����1�� DSFID
���أ�  ִ�н��
*/
uint8_t WriteDSFID(uint8_t DSFID)
{
    uint8_t RSTST;
    uint16_t len;

    if(ISO_SelectFlag == 0)                                                    //��ַģʽ,��UID
    {
        ISO_SendTemp[0x00] = 0x22;
        ISO_SendTemp[0x01] = 0x29;
        memcpy(ISO_SendTemp + 2, ISO_UIDTemp, 0x08);
        ISO_SendTemp[0x0A] = DSFID;

        THM3070_SendFrame_V(ISO_SendTemp, 0x0B);
    }
    else                                                                       //ѡ��ģʽ,����UID
    {
        ISO_SendTemp[0x00] = 0x12;
        ISO_SendTemp[0x01] = 0x29;
        ISO_SendTemp[0x02] = DSFID;

        THM3070_SendFrame_V(ISO_SendTemp, 0x03);
    }
    RSTST = THM3070_RecvFrame_V(ISO_SendTemp, &len);
    if(RSTST == THM_RSTST_FEND && ISO_SendTemp[0x00] != 0x00)
    {
        RSTST = ISO_SendTemp[1];
    }

    return RSTST;
}


/*
���ܣ�  ��ϵͳ��Ϣ
����1�� ϵͳ��Ϣ
����2�� ����
���أ�  ִ�н��
*/
uint8_t ReadSysInfo(uint8_t *InfoData, uint16_t *InfoDataLen)
{
    uint8_t RSTST;
    uint16_t len;

    if(ISO_SelectFlag == 0)                                                    //��ַģʽ,��UID
    {
        ISO_SendTemp[0x00] = 0x22;
        ISO_SendTemp[0x01] = 0x2B;
        memcpy(ISO_SendTemp + 2, ISO_UIDTemp, 0x08);

        THM3070_SendFrame_V(ISO_SendTemp, 0x0A);
    }
    else                                                                       //ѡ��ģʽ,����UID
    {
        ISO_SendTemp[0x00] = 0x12;
        ISO_SendTemp[0x01] = 0x2B;

        THM3070_SendFrame_V(ISO_SendTemp, 0x02);
    }
    RSTST = THM3070_RecvFrame_V(ISO_SendTemp, &len);
    if(RSTST == THM_RSTST_FEND && ISO_SendTemp[0x00] != 0x00)
    {
        RSTST = ISO_SendTemp[1];
    }
    if(RSTST == THM_RSTST_FEND)
    {
        memcpy(InfoData, ISO_SendTemp + 1, len - 1);
        *InfoDataLen = len - 1;
    }

    return RSTST;
}

/*
���ܣ�  ����״̬
����1�� ��״̬
����2�� ����
���أ�  ִ�н��
*/
uint8_t ReadMultipleStatus(uint8_t BlockNum, uint8_t BlockLen, uint8_t *Status, uint16_t *StatusLen)
{
    uint8_t RSTST;
    uint16_t len;

    if(ISO_SelectFlag == 0)                                                    //��ַģʽ,��UID
    {
        ISO_SendTemp[0x00] = 0x22;
        ISO_SendTemp[0x01] = 0x2C;
        memcpy(ISO_SendTemp + 2, ISO_UIDTemp, 0x08);
        ISO_SendTemp[0x0A] = BlockNum;
        ISO_SendTemp[0x0B] = BlockLen;

        THM3070_SendFrame_V(ISO_SendTemp, 0x0C);
    }
    else                                                                       //ѡ��ģʽ,����UID
    {
        ISO_SendTemp[0x00] = 0x12;
        ISO_SendTemp[0x01] = 0x2C;
        ISO_SendTemp[0x02] = BlockNum;
        ISO_SendTemp[0x03] = BlockLen;

        THM3070_SendFrame_V(ISO_SendTemp, 0x04);
    }
    RSTST = THM3070_RecvFrame_V(ISO_SendTemp, &len);
    if(RSTST == THM_RSTST_FEND && ISO_SendTemp[0x00] != 0x00)
    {
        RSTST = ISO_SendTemp[1];
    }
    if(RSTST == THM_RSTST_FEND)
    {
        memcpy(Status, ISO_SendTemp + 1, len - 1);
        *StatusLen = len - 1;
    }

    return RSTST;
}

/*
���ܣ�  ͸������
����1�� ��������
����2�� ����
����3�� ��������
����4�� ����
���أ�  ִ�н��
*/
uint8_t SendRFUCMD(uint8_t *SendData, uint16_t SendDataLen, uint8_t *RecvData, uint16_t *RecvDataLen)
{
    uint8_t RSTST;

    THM3070_WriteREG(THM_REG_PSEL, 0x20);                                      //ͨ��Э����ΪISO15693
    THM3070_WriteREG(THM_REG_SMOD, 0x01);                                      //

    THM3070_SendFrame_V(SendData, SendDataLen);
    RSTST = THM3070_RecvFrame_V(RecvData, RecvDataLen);

    return RSTST;
}



/*
���ܣ�	����������,����
����1��	���
���أ�	ִ�н��
*/
uint8_t LockBlocks(uint8_t BlockNum)
{
    uint8_t RSTST;
    uint16_t len;

    if(ISO_SelectFlag == 0)                                                    //��ַģʽ,��UID
    {
        ISO_SendTemp[0x00] = 0x22;
        ISO_SendTemp[0x01] = 0x22;
        memcpy(ISO_SendTemp + 2, ISO_UIDTemp, 0x08);
        ISO_SendTemp[0x0A] = BlockNum;

        THM3070_SendFrame_V(ISO_SendTemp, 0x0B);
    }
    else                                                                       //ѡ��ģʽ,����UID
    {
        ISO_SendTemp[0x00] = 0x12;
        ISO_SendTemp[0x01] = 0x22;
        ISO_SendTemp[0x02] = BlockNum;

        THM3070_SendFrame_V(ISO_SendTemp, 0x03);
    }
    RSTST = THM3070_RecvFrame_V(ISO_SendTemp, &len);
    if(RSTST == THM_RSTST_FEND && ISO_SendTemp[0x00] != 0x00)
    {
        RSTST = ISO_SendTemp[1];
    }

    return RSTST;
}

/*
���ܣ�  ��������AFI,����
����1�� ���
���أ�  ִ�н��
*/
uint8_t LockAFI()
{
    uint8_t RSTST;
    uint16_t len;

    if(ISO_SelectFlag == 0)                                                    //��ַģʽ,��UID
    {
        ISO_SendTemp[0x00] = 0x22;
        ISO_SendTemp[0x01] = 0x28;
        memcpy(ISO_SendTemp + 2, ISO_UIDTemp, 0x08);

        THM3070_SendFrame_V(ISO_SendTemp, 0x0A);
    }
    else                                                                       //ѡ��ģʽ,����UID
    {
        ISO_SendTemp[0x00] = 0x12;
        ISO_SendTemp[0x01] = 0x28;

        THM3070_SendFrame_V(ISO_SendTemp, 0x02);
    }
    RSTST = THM3070_RecvFrame_V(ISO_SendTemp, &len);
    if(RSTST == THM_RSTST_FEND && ISO_SendTemp[0x00] != 0x00)
    {
        RSTST = ISO_SendTemp[1];
    }

    return RSTST;
}

/*
���ܣ�  ��������DSFID,����
����1�� ���
���أ�  ִ�н��
*/
uint8_t LockDSFID()
{
    uint8_t RSTST;
    uint16_t len;

    if(ISO_SelectFlag == 0)                                                    //��ַģʽ,��UID
    {
        ISO_SendTemp[0x00] = 0x22;
        ISO_SendTemp[0x01] = 0x2A;
        memcpy(ISO_SendTemp + 2, ISO_UIDTemp, 0x08);

        THM3070_SendFrame_V(ISO_SendTemp, 0x0A);
    }
    else                                                                       //ѡ��ģʽ,����UID
    {
        ISO_SendTemp[0x00] = 0x12;
        ISO_SendTemp[0x01] = 0x2A;

        THM3070_SendFrame_V(ISO_SendTemp, 0x02);
    }
    RSTST = THM3070_RecvFrame_V(ISO_SendTemp, &len);
    if(RSTST == THM_RSTST_FEND && ISO_SendTemp[0x00] != 0x00)
    {
        RSTST = ISO_SendTemp[1];
    }

    return RSTST;
}

/*
���ܣ�	���Կ�Ƭ�Ƿ��������߳���
����1��	��
���أ�	ִ�н��
*/
uint8_t TESTV()
{
    uint8_t RSTST;
    uint16_t len;

    if(ISO_SelectFlag == 0)                                                    //��ַģʽ,��UID
    {
        ISO_SendTemp[0x00] = 0x22;
        ISO_SendTemp[0x01] = 0x03;                                             //����һ������δ���������
        memcpy(ISO_SendTemp + 2, ISO_UIDTemp, 0x08);

        THM3070_SendFrame_V(ISO_SendTemp, 0x0A);
    }
    else                                                                       //ѡ��ģʽ,����UID
    {
        ISO_SendTemp[0x00] = 0x12;
        ISO_SendTemp[0x01] = 0x03;

        THM3070_SendFrame_V(ISO_SendTemp, 0x02);
    }
    RSTST = THM3070_RecvFrame_V(ISO_SendTemp, &len);                           //������Ӧ

    return RSTST;
}
