
#include "STM7816.h"
#include "string.h"


/*
�ļ���;:           STM32�����Ӵ���
����:               �Ŷ���
����ʱ��:           2018/07/04
����ʱ��:           2018/07/04
�汾:               V1.0

��ʷ�汾:           V1.0:����STM32 USART��ʵ��7816 T=0Э��


*/




/*��λ����*/
#define STM_RST_H    GPIO_SetBits(STM_RST_GPIOx,STM_RST_Pinx)
#define STM_RST_L    GPIO_ResetBits(STM_RST_GPIOx,STM_RST_Pinx)



uint8_t STM_ATR[40];                                                            //�洢һ��ATR
uint8_t ATR_TA1 = 0x00;                                                         //��ATR��TA1��ֵ,TA1����FD
uint8_t STM_T1 = 0;                                                             //���Ƿ�ΪT=1

uint32_t STM_WT = 9600;                                                         //ͨ�ų�ʱʱ��WT

uint8_t STM_F = 1;                                                              //F
uint8_t STM_D = 1;                                                              //D
uint32_t STM_ClkHz = 3600000;                                                   //Ƶ��3.6MHz

uint16_t STM_DelayMS = 0;                                                       //��ʱ����

                                                                                //FD��
static const uint16_t F_Table[16] = {372, 372, 558, 744, 1116, 1488, 1860, 372, 372, 512, 768, 1024, 1536, 2048, 372, 372};
static const uint8_t D_Table[16] = {1, 1, 2, 4, 8, 16, 32, 64, 12, 20, 1, 1, 1, 1, 1, 1};


/*
���ܣ�  STM7816�ڳ�ʼ��
������  ��
���أ�  ��
*/
void STM7816_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    USART_ClockInitTypeDef USART_ClockInitStructure;

    STM_RCC_APBxPeriphClockCmd;

    GPIO_InitStructure.GPIO_Pin = STM_CLK_Pinx;                                 //CLK����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(STM_CLK_GPIOx, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = STM_IO_Pinx;                                  //IO����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(STM_IO_GPIOx, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = STM_RST_Pinx;                                 //��λ���
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(STM_RST_GPIOx, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = STM_VCC_Pinx;                                 //�������
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(STM_VCC_GPIOx, &GPIO_InitStructure);
    STM7816_SetVCC(1);                                                          //�ϵ�


    USART_SetGuardTime(STM_USARTx, 12);                                         //����ʱ��

    USART_ClockInitStructure.USART_Clock = USART_Clock_Enable;
    USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
    USART_ClockInitStructure.USART_CPHA = USART_CPHA_1Edge;
    USART_ClockInitStructure.USART_LastBit = USART_LastBit_Enable;
    USART_ClockInit(STM_USARTx, &USART_ClockInitStructure);

    STM7816_SetClkHz(STM_ClkHz);                                                //����CLKƵ��

    USART_InitStructure.USART_BaudRate = 9677;
    USART_InitStructure.USART_WordLength = USART_WordLength_9b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1_5;
    USART_InitStructure.USART_Parity = USART_Parity_Even;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(STM_USARTx, &USART_InitStructure);

    STM7816_SetFD(STM_F, STM_D);                                                //����FD

    USART_Cmd(STM_USARTx, ENABLE);
    USART_SmartCardNACKCmd(STM_USARTx, ENABLE);
    USART_SmartCardCmd(STM_USARTx, ENABLE);


}

/*
���ܣ�  ��ʱ������ʱ�ж�(1MS�ж�)
������  ��
���أ�  ��
*/
void STM7816_TIMxInt(void)
{
    if(STM_DelayMS > 0)                                                         //����ʱ��>0
    {
        STM_DelayMS--;                                                          //
    }
}

/*
���ܣ�  STM7816������Ƶ��
������  Ƶ��,=0Ϊ��ʱ��
���أ�  ��
*/
void STM7816_SetClkHz(uint32_t hz)
{
    uint32_t apbclock = 0x00;
    uint32_t usartxbase = 0;
    RCC_ClocksTypeDef RCC_ClocksStatus;

    if(hz == 0)
    {
        STM_USARTx->CR2 &= ~0x00000800;                                         //��ʱ��

        return;
    }

    usartxbase = (uint32_t)STM_USARTx;
    RCC_GetClocksFreq(&RCC_ClocksStatus);
    if (usartxbase == USART1_BASE)
    {
        apbclock = RCC_ClocksStatus.PCLK2_Frequency;
    }
    else
    {
        apbclock = RCC_ClocksStatus.PCLK1_Frequency;
    }

    apbclock /= hz;                                                             //���ݴ��ڵ�Ƶ�ʼ����Ƶ��
    apbclock /= 2;
    if(apbclock < 1)apbclock = 1;

    USART_SetPrescaler(STM_USARTx, apbclock);                                   //���÷�Ƶ
    STM_USARTx->CR2 |= 0x00000800;                                              //��ʱ��

}


/*
���ܣ�  STM7816������FD
������  FD�ı������ֵ
���أ�  ��
*/
void STM7816_SetFD(uint8_t F, uint8_t D)
{
    uint32_t etudiv;

    etudiv = STM_USARTx->GTPR & 0x0000001F;                                     //��ȡʱ�ӷ�Ƶ��
    etudiv = 2 * etudiv * F_Table[F] / D_Table[D];                              //���ر���=((ʱ�ӷ�Ƶ*2)*F)/D

    STM_USARTx->BRR = etudiv;
}

/*
���ܣ�  STM7816�ӿ�����ͨ�ų�ʱʱ��
������  ��ʱʱ��(��λETU)
���أ�  ��
*/
void STM7816_SetWT(uint32_t wt)
{
    STM_WT = wt;
}

/*
���ܣ�  ���ڽ���һ���ֽ�����
������  ����,��ʱʱ��(��λMS)
���أ�  1��ʱ����,0�ɹ�
*/
static uint8_t USART_RecvByte(uint8_t *dat, uint16_t overMs)
{
    STM_DelayMS = overMs + 1;                                                   //���ó�ʱʱ��,+1����1ms���
    while(STM_DelayMS)                                                          //ʱ����
    {
        //���յ�����
        if(RESET != USART_GetFlagStatus(STM_USARTx, USART_FLAG_RXNE))
        {
            *dat = (uint8_t)USART_ReceiveData(STM_USARTx);
            break;
        }
    }
    overMs = STM_DelayMS == 0;                                                  //�Ƿ�ʱ
    STM_DelayMS = 0;

    return overMs;                                                              //����
}

/*
���ܣ�  ���ڷ���һ���ֽ�����
������  ����
���أ�  ��
*/
static void USART_SendByte(uint8_t dat)
{
    USART_ClearFlag(STM_USARTx, USART_FLAG_TC);                                 //�巢�ͱ�ʶ
    USART_SendData(STM_USARTx, dat);                                            //����
    while(USART_GetFlagStatus(STM_USARTx, USART_FLAG_TC) == RESET);
    //while(USART_GetFlagStatus(STM_USARTx,USART_FLAG_RXNE)==RESET);            //û����ʱ���һֱ�ȴ�
    (void)USART_ReceiveData(STM_USARTx);                                        //ΪʲôҪ����?
}
/*
���ܣ�  CLK��MSת��,�ڲ�����
������  CLK,1��һ/0��ȥ
���أ�  ת�����MSֵ
*/
static uint16_t CLKToMS(uint32_t clk, uint8_t half)
{
    uint16_t temp;

    temp = clk / (STM_ClkHz / 1000);                                           //����
    clk = clk % (STM_ClkHz / 1000);                                            //����

    if(half && clk)                                                            //���������ҽ�һ
    {
        temp += 1;                                                             //��һ
    }
    return temp;                                                               //����
}
/*
���ܣ�  CLK��USת��,�ڲ�����
������  CLK,1��һ/0��ȥ
���أ�  ת�����USֵ
*/
static uint16_t CLKToUS(uint32_t clk, uint8_t half)
{
    uint16_t temp;

    clk *= 1000;
    temp = clk / (STM_ClkHz / 1000);                                            //����
    clk = clk % (STM_ClkHz / 1000);                                             //����

    if(half && clk)                                                             //���������ҽ�һ
    {
        temp += 1;                                                              //��һ
    }
    return temp;                                                                //����
}
/*
���ܣ�  ETU��MSת��,�ڲ�����
������  ETU,1��һ/0��ȥ
���أ�  ת�����MSֵ
*/
static uint16_t ETUToMS(uint32_t etu, uint8_t half)
{
    uint16_t temp;

    etu *= (F_Table[STM_F] / D_Table[STM_D]);
    temp = etu / (STM_ClkHz / 1000);                                            //����
    etu = etu % (STM_ClkHz / 1000);                                             //����

    if(half && etu)                                                             //���������ҽ�һ
    {
        temp += 1;                                                              //��һ
    }
    return temp;                                                                //����
}

/*
���ܣ�  ����һ������1(2����)�ĸ���,�ڲ�����
������  ����
���أ�  1�ĸ���
*/
static uint8_t NumberOf1_Solution1(uint32_t num)
{
    uint8_t count = 0;
    while(num)
    {
        if(num & 0x01)
        {
            count++;
        }
        num = num >> 1;
    }
    return count;
}

/*
���ܣ�  Ԥ��ATR����������,�ڲ�����
������  ��֪��ATR���ݼ�����
���أ�  ATR����������
*/
uint8_t foreATRLen(uint8_t *atr, uint8_t len)
{
    uint8_t len1 = 2, len2 = 2, next = 1, temp = 1, TD2;

    STM_T1 = 0;                                                                 //Ĭ��T=0
    ATR_TA1 = 0;                                                                //TA1����
    if(len < len2)                                                              //������̫С
    {
        return 0xFF;
    }
    while(next)
    {
        next = 0;
        if((atr[len2 - 1] & 0x80) == 0x80)                                      //TDi����
        {
            next = 1;                                                           //��������ѭ��
            len1++;
        }
        if((atr[len2 - 1] & 0x40) == 0x40)                                      //TCi����
        {
            len1++;
        }
        if((atr[len2 - 1] & 0x20) == 0x20)                                      //TBi����
        {
            len1++;
        }
        if((atr[len2 - 1] & 0x10) == 0x10)                                      //TAi����
        {
            len1++;

            if(len2 == 2)                                                       //TA1����
            {
                ATR_TA1 = atr[2];                                               //��¼TA1
            }
        }
        len2 = len1;
        if(len < len2)                                                          //������̫С
        {
            return 0xFF;
        }
    }
    len1 += (atr[1] & 0x0F);                                                    //������ʷ�ֽڳ���

    if(atr[1] & 0x80)                                                           //TD1����
    {
        temp += NumberOf1_Solution1(atr[1] & 0xF0);                             //ͨ���ж�1��λ��,�ҵ�TD1������
        if((atr[temp] & 0x0F) == 0x01)                                          //TD1��λΪ1,��ΪT=1��
        {
            STM_T1 = 1;
        }
        if(atr[temp] & 0x80)                                                    //�ж�TD2�Ƿ����
        {
            STM_T1 = 1;                                                         //����һ����T=1
            temp += NumberOf1_Solution1(atr[temp] & 0xF0);                      //ͨ���ж�1��λ��,�ҵ�TD2������
            TD2 = temp;
            if((atr[TD2] & 0x30) == 0x30)
            {
                STM_T1 = 1;
            }
            else if((atr[TD2] & 0xF0) == 0x00 && (atr[TD2] & 0x0F) == 0x01)
            {
                STM_T1 = 1;
            }
        }
    }
    if(STM_T1)                                                                  //T=1ATR�����1�ֽ�У��
    {
        len1++;
    }
    return len1;                                                                //����Ԥ���ATR����ֵ
}

/*
���ܣ�  �ȸ�λ
������  ATR,ATR����
���أ�  1��ʱ,0�ɹ�
*/
uint8_t WarmReset(uint8_t *atr, uint16_t *len)
{
    uint8_t i, err;
    uint16_t overTim;

    USART_RecvByte(&i, 1);                                                      //�����õ��������

    STM_WT = 9600;                                                              //�ָ���ʼֵ
    STM_F = 1;
    STM_D = 1;
    *len = 0;

    STM7816_SetFD(STM_F, STM_D);                                                //�ָ�������

    STM_RST_L;                                                                  //��λ������
    overTim = CLKToUS(500, 1);                                                  //CLK��US,��һ��֤����
    delay_us(overTim);                                                          //�ȴ�����400��ʱ��

    STM_RST_H;                                                                  //��λ������
    overTim = CLKToUS(300, 0);                                                  //CLK��US,����һ��֤С��
    delay_us(overTim);                                                          //�ȴ����400��ʱ��

    for(i = 0; i < sizeof(STM_ATR); i++)                                        //ѭ������ATR
    {
        if(i == 0)
        {
            overTim = CLKToMS(40000, 1);                                        //��һ���ֽڷ���ʱ����400-40000��ʱ����
        }
        else
        {
            overTim = ETUToMS(STM_WT * D_Table[STM_D], 1);                      //�����ֽڰ���ʱʱ��
        }
        err = USART_RecvByte(STM_ATR + i, overTim);                             //�ڳ�ʱʱ���ڽ���һ�ֽ�
        if(!err)                                                                //���յ�
        {
            uint8_t atrLen;

            (*len)++;
            atrLen = foreATRLen(STM_ATR, *len);
            if(atrLen == *len)
            {
                break;
            }
        }
        else                                                                    //��ʱ
        {
            err = i == 0;
            break;
        }
    }
    memcpy(atr, STM_ATR, *len);                                                 //����ATR����


    return err;
}

/*
���ܣ�  ����IO��״̬,�ڲ�����
������  1��/0��
���أ�  ��
*/
static void setIOState(uint8_t on)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    if(on)
    {
        GPIO_InitStructure.GPIO_Pin = STM_IO_Pinx;                              //IO����
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(STM_IO_GPIOx, &GPIO_InitStructure);
    }
    else
    {
        GPIO_InitStructure.GPIO_Pin = STM_IO_Pinx;                              //IO���
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(STM_IO_GPIOx, &GPIO_InitStructure);

        GPIO_ResetBits(STM_IO_GPIOx, STM_IO_Pinx);                              //����
    }
}

/*
���ܣ�  ����VCC
������  1��/0��
���أ�  ��
*/
void STM7816_SetVCC(uint8_t on)
{
    if(on)
    {
        GPIO_SetBits(STM_VCC_GPIOx, STM_VCC_Pinx);                              //����
    }
    else
    {
        GPIO_ResetBits(STM_VCC_GPIOx, STM_VCC_Pinx);                            //����
    }
}

/*
���ܣ�  �临λ
������  ATR,ATR����
���أ�  1��ʱ,0�ɹ�
*/
uint8_t ColdReset(uint8_t *atr, uint16_t *len)
{
    uint8_t i, err;
    uint16_t overTim;

    USART_RecvByte(&i, 1);                                                      //�����õ��������

    STM_WT = 9600;                                                              //�ָ���ʼֵ
    STM_F = 1;
    STM_D = 1;
    *len = 0;

    STM7816_SetFD(STM_F, STM_D);                                                //�ָ�������

    STM7816_SetClkHz(0);                                                        //CLK,IO,RST����
    setIOState(0);
    STM_RST_L;

    STM7816_SetVCC(0);                                                          //�ϵ�
    delay_ms(50);                                                               //��ʱ

    STM7816_SetVCC(1);                                                          //�ϵ�
    delay_ms(50);                                                               //��ʱ

    STM7816_SetClkHz(STM_ClkHz);                                                //��ʱ��
    setIOState(1);                                                              //200��ʱ����IO����,�����ڸ�ʱ�Ӻ�ֱ������

    overTim = CLKToUS(500, 1);                                                  //CLK��US,��һ��֤����
    delay_us(overTim);                                                          //�ȴ�����400��ʱ��
    STM_RST_H;                                                                  //��λ������
    overTim = CLKToUS(300, 0);                                                  //CLK��US,����һ��֤С��
    delay_us(overTim);                                                          //�ȴ����400��ʱ��,����Ҫ��,�п�����RST���ߺ����Ϸ���һ���ֽ�����,�����

    for(i = 0; i < sizeof(STM_ATR); i++)                                        //ѭ������ATR
    {
        if(i == 0)
        {
            overTim = CLKToMS(40000, 1);                                        //��һ���ֽڷ���ʱ����400-40000��ʱ����
        }
        else
        {
            overTim = ETUToMS(STM_WT * D_Table[STM_D], 1);                      //�����ֽڰ���ʱʱ��
        }
        err = USART_RecvByte(STM_ATR + i, overTim);                             //�ڳ�ʱʱ���ڽ���һ�ֽ�
        if(!err)                                                                //���յ�
        {
            uint8_t atrLen;

            (*len)++;
            atrLen = foreATRLen(STM_ATR, *len);
            if(atrLen == *len)
            {
                break;
            }
        }
        else                                                                    //��ʱ
        {
            err = i == 0;
            break;
        }
    }
    memcpy(atr, STM_ATR, *len);                                                 //����ATR����


    return err;

}

/*
���ܣ�  ����APDU�����շ�������
������  ��������,����,��������,����
���أ�  0�ɹ�,1��ʱ,2APDU��ʽ��,3ͨ�Ŵ�
*/
uint8_t ExchangeTPDU(uint8_t *sData, uint16_t len_sData, uint8_t *rData, uint16_t *len_rData)
{
    uint8_t err, wait, recvFlag;
    uint8_t i, lc, le;

    uint8_t pc;
    uint8_t INS = sData[1];                                                     //��¼һ��INS,��ֹ���յ�����֮��ʹ��ʱ�����ǳ���

    uint16_t overTim;
    overTim = ETUToMS(STM_WT * D_Table[STM_D], 1);                              //��ʱʱ��

    if(len_sData == 4)                                                          //����Ϊ4,CASE1
    {
        sData[4] = 0x00;
        lc = le = 0;
    }
    else if(len_sData == 5)                                                     //����Ϊ5,CASE2
    {
        lc = 0;
        le = sData[4];
    }
    else
    {
        if(len_sData == sData[4] + 5)                                           //Lc+5,CASE3
        {
            lc = sData[4];
            le = 0;
        }
        else if(len_sData == sData[4] + 6)                                      //Lc+5+1(Le),CASE4
        {
            lc = sData[4];
            le = sData[len_sData - 1];
        }
        else
        {
            return 2;                                                           //�޷�����APDU
        }
    }

    USART_RecvByte(&i, 1);                                                      //������õ�����
    for(i = 0; i < 5; i++)                                                      //����5��APDUͷ
    {
        USART_SendByte(sData[i]);
    }

    wait = 1;
    len_sData = 0;
    *len_rData = 0;
    recvFlag = 0;
    while(wait)                                                                 //��Ҫ�����ȴ�
    {
        wait = 0;
        err = USART_RecvByte(&pc, overTim);                                     //����һ�ֽ�����
        if(err)
        {
            return err;                                                         //����
        }
        else
        {
            if((pc >= 0x90 && pc <= 0x9F) || (pc >= 0x60 && pc <= 0x6F))        //����90-9F/60-6F֮��
            {
                switch(pc)
                {
                case 0x60:                                                      //�����ȴ�
                    wait = 1;
                    break;
                default:                                                        //״̬��SW
                    rData[*len_rData] = pc;                                     //SW1
                    (*len_rData)++;
                    USART_RecvByte(&pc, overTim);
                    rData[*len_rData] = pc;										//SW2
                    (*len_rData)++;
                    break;
                }
            }
            else                                                                //ACK
            {
                pc ^= INS;                                                      //���������INS
                if(pc == 0)                                                     //����ֵ=INS,��ʶ������Ӧ�ý�����Ҫ���͵����ݷ���,��׼������ȫ������
                {
                    if(recvFlag == 0 && lc > len_sData)                         //����״̬����������Ҫ����
                    {
                        for(i = 0; i < lc - len_sData; i++)                     //����Ҫ���͵�ȫ������
                        {
                            USART_SendByte(sData[i + 5 + len_sData]);
                        }
                        len_sData = lc;
                        recvFlag = 1;                                           //����״̬
                    }
                    if((recvFlag == 1 || lc == 0) && le > *len_rData)           //(����״̬��û������Ҫ����)����������Ҫ����
                    {
                        for(i = 0; i < le - *len_rData; i++)
                        {
                            err = USART_RecvByte(rData + *len_rData + i, overTim);
                            if(err && i < le - *len_rData)
                            {
                                *len_rData = i;
                                return err;                                    //����
                            }
                        }
                        *len_rData = le;
                    }
                    wait = 1;
                }
                else if(pc == 0xFF)                                             //����ֵ=~INS,��ʶ����������һ�ֽ�����,��׼������1�ֽ�����
                {
                    if(recvFlag == 0 && lc > len_sData)                         //����״̬����������Ҫ����
                    {
                        USART_SendByte(sData[5 + len_sData]);                   //���ͽ�������һ�ֽ�����
                        len_sData++;
                        if(len_sData == lc)                                     //�������
                        {
                            recvFlag = 1;                                       //����״̬
                        }
                    }
                    if((recvFlag == 1 || lc == 0) && le > *len_rData)           //(����״̬��û������Ҫ����)����������Ҫ����
                    {
                        err = USART_RecvByte(rData + *len_rData, overTim);      //����һ�ֽ�����
                        if(err)
                        {
                            break;
                        }
                        (*len_rData)++;                                         //���ճ����ۼ�
                    }
                    wait = 1;
                }
                else                                                            //����
                {
                    return 3;
                }
            }
        }
    }
    return 0;
}

/*
���ܣ�  PPS
������  F,D�ı������ֵ
���أ�  0�ɹ�,1ʧ��
*/
uint8_t PPS(uint8_t F, uint8_t D)
{
    uint8_t i, err;
    uint16_t overTim;
    uint8_t pps_cmd[4] = {0xFF, 0x10, 0xFD, 0x00};
    uint8_t pps_res[4] = {0x00, 0x00, 0x00, 0x00};

    overTim = ETUToMS(STM_WT * D_Table[STM_D], 1);                              //��ʱʱ��

    pps_cmd[2] = ((F << 4) & 0xF0) | (D & 0x0F);                                //��ֵFD
    for(i = 0; i < 3; i++)
    {
        pps_cmd[3] ^= pps_cmd[i];                                               //���У��
    }

    for(i = 0; i < 4; i++)                                                      //����PPS
    {
        USART_SendByte(pps_cmd[i]);
    }

    for(i = 0; i < 4; i++)
    {
        err = USART_RecvByte(&pps_res[i], overTim);
        if(err)
        {
            break;                                                              //����
        }
    }
    if(i == 4)                                                                  //��Э��Ӧ�������ֽ�
    {
        if(pps_res[0] == 0xFF && (pps_res[1] & 0x10) == 0x10 && pps_res[2] == pps_cmd[2])
        {
            STM7816_SetFD(F, D);
        }
        else
        {
            return 1;
        }
    }
    else                                                                        //����Э��
    {
        //
        //if(((pps_res[0]&0x10)==0x10&&pps_res[1]==pps_cmd[2]))
        //{
            //STM7816_SetFD(F,D);
        //}
        //else
        //{
            //return 1;
        //}
    }

    return 1;
}







