

#include "THM3070.h"


/*
�ļ���;:           THM3070�����ļ�
����:               �Ŷ���
����ʱ��:           2018/04/20
����ʱ��:           2018/05/31
�汾:               V1.1

��ʷ�汾:           V1.0:ʵ��THM3070����,ͨ�Žӿ�Ϊ���ģ��SPI��ʽ
                    V1.1:����Ӳ��SPI,���Ӳ����ͨ��THM_SPIMODѡ��
*/





#if THM_SPIMOD==0
/*ģ��SPI����״̬�л�����*/
#define THM_IO1_SCK_H       GPIO_SetBits(THM_IO1_SCK_GPIOx,THM_IO1_SCK_Pinx)
#define THM_IO1_SCK_L       GPIO_ResetBits(THM_IO1_SCK_GPIOx,THM_IO1_SCK_Pinx)
#define THM_IO2_MOSI_H      GPIO_SetBits(THM_IO2_MOSI_GPIOx,THM_IO2_MOSI_Pinx)
#define THM_IO2_MOSI_L      GPIO_ResetBits(THM_IO2_MOSI_GPIOx,THM_IO2_MOSI_Pinx)

#define THM_IO3_MISO        GPIO_ReadInputDataBit(THM_IO3_MISO_GPIOx,THM_IO3_MISO_Pinx)

/*ģ��SPI����״̬�л�����*/
#endif

#define THM_IO4_SS_N_H      GPIO_SetBits(THM_IO4_SS_N_GPIOx,THM_IO4_SS_N_Pinx)
#define THM_IO4_SS_N_L      GPIO_ResetBits(THM_IO4_SS_N_GPIOx,THM_IO4_SS_N_Pinx)


#define THM3070_TimeCountMax 0x00004400                                         //���ȴ�����,��Ӧ����,��ֹTHM3070���շ�֡����ʱ����
static uint32_t THM3070_TimeCount = 0;


static void THM3070_GPIO_Init(void);
static uint8_t THM3070_REG_Init(void);
/*
���ܣ�  THM3070оƬ��ʼ��
������  ��
���أ�  1�ɹ���0ʧ��
*/
uint8_t THM3070_Init()
{
    uint8_t temp;

    THM3070_GPIO_Init();                                                        //���ų�ʼ��

    GPIO_ResetBits(THM_RESET_GPIOx, THM_RESET_Pinx);
    delay_us(5000);                                                             //��λ
    GPIO_SetBits(THM_RESET_GPIOx, THM_RESET_Pinx);

    temp = THM3070_REG_Init();                                                  //�Ĵ�����ʼ��

    //THM3070_RFClose();                                                         //Ĭ�ϰѳ��ر�

    return temp;
}

/*
���ܣ�  THM3070����GPIO��ʼ�����ڲ�����
������  ��
���أ�  ��
*/
static void THM3070_GPIO_Init()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    THM_RCC_APBxPeriphClockCmd;                                                 //��ʱ��

    GPIO_InitStructure.GPIO_Pin = THM_MOD0_Pinx;                                //MOD0�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(THM_MOD0_GPIOx, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = THM_STANDBY_Pinx;                             //STANDBY�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(THM_STANDBY_GPIOx, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = THM_RESET_Pinx;                               //RESET�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(THM_RESET_GPIOx, &GPIO_InitStructure);

    THM3070_SleepMode(0);                                                       //����
    THM3070_PortMode(0);                                                        //SPIͨ�Žӿ�

}
/*
���ܣ�  THM3070�Ĵ�����ʼ�����ڲ�����
������  ��
���أ�  ��
*/
static uint8_t THM3070_REG_Init()
{
    uint8_t temp;

    THM3070_WriteREG(THM_REG_TXDP1, 0xC8);                                      //��ǿ��С,TYPEB ģʽ�±�֤DP0>0 AND DP0<DP1
    THM3070_WriteREG(THM_REG_TXDP0, 0xC0);                                      //
    THM3070_WriteREG(THM_REG_TXDN1, 0x60);                                      //
    THM3070_WriteREG(THM_REG_TXDN0, 0x17);                                      //TYPEB�������200/DP0:200/DN0,0x17 is default


    THM3070_WriteREG(THM_REG_PSEL, 0x00);                                       //TYPEB,106kbit/s
    THM3070_WriteREG(THM_REG_FCONB, 0x2A);                                      //EOF=10.5ETU,SOF_H=2.5ETU,SOF_L=10.5ETU
    THM3070_WriteREG(THM_REG_EGT, 0x01);                                        //EGT=2ETU
    THM3070_WriteREG(THM_REG_CRCSEL, 0xC1);                                     //����CRCʹ��,����CRC�ж�ʹ��,���ճ�ʱ��ʱʹ��
    THM3070_WriteREG(THM_REG_INTCON, 0x01);                                     //����IRQ���
    THM3070_WriteREG(THM_REG_SMOD, 0x00);                                       //����1/4,�������10%-30%
    THM3070_WriteREG(THM_REG_PWYH, 0x27);                                       //(0x27+1)/Fc
    THM3070_WriteREG(THM_REG_EMVEN, 0xFD);                                      //ʹ�ܸ�������������
    THM3070_WriteREG(THM_REG_RXCON, 0x42);                                      //���յ�·����,�Ŵ���40dB
    THM3070_WriteREG(THM_REG_TXCON, 0x62);                                      //10%ASK����,�򿪷����·

    THM3070_SetFWT(0x64);                                                       //����һ��Ĭ�ϵĳ�ʱʱ��100*330us=33ms

    temp = THM3070_ReadREG(THM_REG_RXCON);
    if(temp != 0x42)                                                            //���Ĵ����ȽϿ�д���Ƿ�ɹ�
    {
        return 0;
    }
    return 1;
}


static void THM3070SPI_SendBuff(uint8_t *p_val, uint16_t len_val);
static void THM3070SPI_RecvBuff(uint8_t *p_val, uint16_t len_val);
/*
���ܣ�  THM3070д�Ĵ���
����1�� �Ĵ�����ַ
����2�� �Ĵ���ֵ
���أ�  ��
*/
void THM3070_WriteREG(uint8_t addr, uint8_t val)
{
    uint8_t temp[2];

    temp[0] = addr | 0x80;                                                      //д�Ĵ���ʱ��λ��1
    temp[1] = val;

    THM_IO4_SS_N_L;                                                             //Ƭѡ����ʹ��
    THM3070SPI_SendBuff(temp, 2);                                               //���͵�ַ
    THM_IO4_SS_N_H;                                                             //Ƭѡ����

}
/*
���ܣ�  THM3070���Ĵ���
����1�� �Ĵ�����ַ
���أ�  �Ĵ���ֵ
*/
uint8_t THM3070_ReadREG(uint8_t addr)
{
    uint8_t temp[1];

    THM_IO4_SS_N_L;                                                             //Ƭѡ����ʹ��

    temp[0] = addr & 0x7F;                                                      //���Ĵ���ʱ��λ����
    THM3070SPI_SendBuff(temp, 1);                                               //���͵�ַ
    THM3070SPI_RecvBuff(temp, 1);                                               //��������

    THM_IO4_SS_N_H;                                                             //Ƭѡ����

    return temp[0];                                                             //����
}



/*
���ܣ�	SPI����һ���ֽ�
������	�����͵�ֵ
���أ�	��
*/
static void THM3070SPI_SendByte(uint8_t val)
{
#if THM_SPIMOD==0

    uint8_t i;

    for(i = 0; i < 8; i++)                                                      //ѭ����λ����
    {
        THM_IO1_SCK_L;                                                          //ʱ������
        if(val & 0x80)                                                          //�ȷ���λ
        {
            THM_IO2_MOSI_H;
        }
        else                                                                    //�������߻�����
        {
            THM_IO2_MOSI_L;
        }
        val = val << 1;                                                         //��λ
        THM_IO1_SCK_H;                                                          //ʱ������
    }
    THM_IO1_SCK_L;                                                              //ʱ������

#else

    while(SPI_I2S_GetFlagStatus(THM_SPIx, SPI_I2S_FLAG_TXE) == RESET) {;}
    SPI_I2S_SendData(THM_SPIx, val);
    while(SPI_I2S_GetFlagStatus(THM_SPIx, SPI_I2S_FLAG_RXNE) == RESET) {;}
    SPI_I2S_ReceiveData(THM_SPIx);

#endif
}
/*
���ܣ�  SPI�������ݶ�
����1�� �����͵�����ָ��
����2�� �����͵����ݳ���
���أ�  ��
*/
static void THM3070SPI_SendBuff(uint8_t *p_val, uint16_t len_val)
{
    uint16_t i;

    for(i = 0; i < len_val; i++)
    {
        THM3070SPI_SendByte(p_val[i]);                                          //�����ֽ�
    }
}
/*
���ܣ�  SPI����һ���ֽ�
������  ��
���أ�  ���յ���ֵ
*/
static uint8_t THM3070SPI_RecvByte()
{
#if THM_SPIMOD==0
    uint8_t i, dat, temp;

    THM_IO1_SCK_L;                                                              //ʱ������
    dat = 0;
    temp = 0x80;
    for(i = 0; i < 8; i++)                                                      //ѭ����λ����
    {
        THM_IO1_SCK_H;                                                          //ʱ������
        if(THM_IO3_MISO & 0x01)                                                 //��ȡ�źŵ�ƽ
        {
            dat |= temp;
        }
        THM_IO1_SCK_L;                                                          //ʱ������
        temp = temp >> 1;                                                       //��λ
    }
    return dat;                                                                 //����

#else

    while(SPI_I2S_GetFlagStatus(THM_SPIx, SPI_I2S_FLAG_TXE) == RESET) {;}
    SPI_I2S_SendData(THM_SPIx, 0xFF);
    while(SPI_I2S_GetFlagStatus(THM_SPIx, SPI_I2S_FLAG_RXNE) == RESET) {;}
    return SPI_I2S_ReceiveData(THM_SPIx);

#endif
}
/*
���ܣ�  SPI�������ݶ�
����1�� ������ݵ�����ָ��
����2�� Ҫ���յ����ݳ���
���أ�  ��
*/
static void THM3070SPI_RecvBuff(uint8_t *p_val, uint16_t len_val)
{
    uint16_t i;

    if(len_val == 0)                                                            //���ݳ���Ϊ0
    {
        len_val = 0x0100;                                                       //����Ϊ256
    }
    for(i = 0; i < len_val; i++)
    {
        p_val[i] = THM3070SPI_RecvByte();                                       //�����ֽ�
    }
}


/*
���ܣ�  ����֡
����1�� ������ݵ�����ָ��
����2�� Ҫ���͵����ݳ���
���أ�  ��
*/
void THM3070_SendFrame(uint8_t *p_buff, uint16_t len_buff)
{
    uint8_t temp[1];

    THM3070_WriteREG(THM_REG_SCON, 0x05);                                       //�����ݻ�����
    THM3070_WriteREG(THM_REG_SCON, 0x01);                                       //����״̬
    THM3070_WriteREG(THM_REG_EMVERR, 0xFF);                                     //

    temp[0] = THM_REG_DATA | 0x80;                                              //д�Ĵ�����λ��1

    THM_IO4_SS_N_L;                                                             //Ƭѡ����ʹ��

    THM3070SPI_SendBuff(temp, 1);                                               //���͵�ַ
    THM3070SPI_SendBuff(p_buff, len_buff);                                      //��������

    THM_IO4_SS_N_H;                                                             //Ƭѡ����

    THM3070_WriteREG(THM_REG_SCON, 0x03);                                       //��������
    THM3070_TimeCount = 0;
    while(!THM3070_ReadREG(THM_REG_TXFIN))                                      //�ȴ��������
    {
        THM3070_TimeCount++;
        if(THM3070_TimeCount > THM3070_TimeCountMax)                            //���볬ʱ�ж�,��������
        {
            THM3070_TimeCount = 0;
            break;
        }
    }

}

/*
���ܣ�	����֡
����1��	������ݵ�����ָ��
����2��	Ҫ���յ����ݳ���
���أ�	ִ�н��
*/
uint8_t THM3070_RecvFrame(uint8_t *p_buff, uint16_t *len_buff)
{
    uint8_t EMVError, RStatus = 0;

    THM3070_TimeCount = 0;
    while(1)                                                                    //�ȴ�������ϻ����
    {
        EMVError = THM3070_ReadREG(THM_REG_EMVERR);
        RStatus = THM3070_ReadREG(THM_REG_RSTAT);
        if(RStatus)
        {
            if(EMVError & 0x02)
            {
                return THM_ERROR_EMVERR;
            }
            break;
        }
        THM3070_TimeCount++;
        if(THM3070_TimeCount > THM3070_TimeCountMax)                            //���볬ʱ�ж�,��������
        {
            THM3070_TimeCount = 0;
            RStatus = THM_RSTST_TMROVER;
            break;
        }
    }
    if(RStatus & 0x40)                                                          //�жϴ�������
    {
        RStatus = THM_RSTST_CERR;
        //		delay_ms(6);                                                    //��ͻ֮�����ϲ�Ӧ������ʱ
    }
    else if(RStatus & 0x20)
    {
        RStatus = THM_RSTST_PERR;
    }
    else if(RStatus & 0x10)
    {
        RStatus = THM_RSTST_FERR;
    }
    else if(RStatus & 0x08)
    {
        RStatus = THM_RSTST_DATOVER;
    }
    else if(RStatus & 0x04)
    {
        RStatus = THM_RSTST_TMROVER;
    }
    else if(RStatus & 0x02)
    {
        RStatus = THM_RSTST_CRCERR;
    }
    else
    {
        RStatus = THM_RSTST_FEND;                                               //�޴���
    }

    *len_buff = THM3070_ReadREG(THM_REG_RSCH);
    *len_buff = *len_buff << 8;
    *len_buff |= THM3070_ReadREG(THM_REG_RSCL);                                 //��ȡ���ݳ���
    if(*len_buff > 256)
    {
        return THM_RSTST_DATOVER;
    }

    if(*len_buff > 0)
    {
        uint8_t temp[1];

        THM_IO4_SS_N_L;                                                         //Ƭѡ����ʹ��

        temp[0] = THM_REG_DATA;                                                 //д�Ĵ�����λ��1

        THM3070SPI_SendBuff(temp, 1);                                           //���͵�ַ
        THM3070SPI_RecvBuff(p_buff, *len_buff);                                 //��������

        THM_IO4_SS_N_H;                                                         //Ƭѡ����
    }
    THM3070_WriteREG(THM_REG_RSTAT, 0x00);                                      //���״̬

    return RStatus;
}



/*
���ܣ�  �س��ٿ���
������  ��
���أ�  ��
*/
void THM3070_RFReset()
{
    uint8_t temp;

    temp = THM3070_ReadREG(THM_REG_TXCON);                                      //�ȶ�ȡ

    THM3070_WriteREG(THM_REG_TXCON, temp | 0x01);                               //ֻ�����λ��1���ر�
    delay_ms(2);                                                                //��ʱ
    THM3070_WriteREG(THM_REG_TXCON, temp & 0xFE);                               //ֻ�����λ��0����
    delay_ms(8);                                                                //��ʱ,10ms�������׵�
}

/*
���ܣ�	�س��ٿ���
������	��
���أ�	��
*/
void THM3070_RFClose()
{
    uint8_t temp;

    temp = THM3070_ReadREG(THM_REG_TXCON);                                      //�ȶ�ȡ

    THM3070_WriteREG(THM_REG_TXCON, temp | 0x01);                               //ֻ�����λ��1���ر�
}

/*
���ܣ�  ѡ��ӿ�
������  0=SPI,1=IDR
���أ�  ��
*/
void THM3070_PortMode(uint8_t mode)
{
#if THM_SPIMOD==1
    SPI_InitTypeDef SPI_InitStructure;
#endif
    GPIO_InitTypeDef GPIO_InitStructure;

    if(mode)
    {
        GPIO_InitStructure.GPIO_Pin = THM_IO1_SCK_Pinx;                         //����
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(THM_IO1_SCK_GPIOx, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = THM_IO2_MOSI_Pinx;                        //����
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(THM_IO2_MOSI_GPIOx, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = THM_IO3_MISO_Pinx;                        //����
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(THM_IO3_MISO_GPIOx, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = THM_IO4_SS_N_Pinx;                        //����
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(THM_IO4_SS_N_GPIOx, &GPIO_InitStructure);

        GPIO_SetBits(THM_MOD0_GPIOx, THM_MOD0_Pinx);                            //���ߣ�ѡ��IDR�ӿ�ģʽ
    }
    else
    {
#if THM_SPIMOD==0
        GPIO_InitStructure.GPIO_Pin = THM_IO1_SCK_Pinx;                         //SCK�������
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(THM_IO1_SCK_GPIOx, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = THM_IO2_MOSI_Pinx;                        //MOSI�������
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(THM_IO2_MOSI_GPIOx, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = THM_IO3_MISO_Pinx;                        //MISO��������
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(THM_IO3_MISO_GPIOx, &GPIO_InitStructure);

#else
        GPIO_InitStructure.GPIO_Pin = THM_IO1_SCK_Pinx;                         //SCK���Ÿ���
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(THM_IO1_SCK_GPIOx, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = THM_IO2_MOSI_Pinx;                        //MOSI���Ÿ���
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(THM_IO2_MOSI_GPIOx, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = THM_IO3_MISO_Pinx;                        //MISO���Ÿ���
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(THM_IO3_MISO_GPIOx, &GPIO_InitStructure);

        SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
        SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
        SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
        SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
        SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
        SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
        SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
        SPI_InitStructure.SPI_CRCPolynomial = 7;
        SPI_Init(THM_SPIx, &SPI_InitStructure);

        SPI_Cmd(THM_SPIx, ENABLE);
#endif
        GPIO_InitStructure.GPIO_Pin = THM_IO4_SS_N_Pinx;                        //SS_N�������
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(THM_IO4_SS_N_GPIOx, &GPIO_InitStructure);

        THM_IO4_SS_N_H;                                                         //Ƭѡ���ߣ�Ĭ�ϲ�ʹ�ܶ�д

        GPIO_ResetBits(THM_MOD0_GPIOx, THM_MOD0_Pinx);                          //���ͣ�ѡ��SPIͨ��
    }
}
/*
���ܣ�  ���л��ߵ͹���
������  0=����,1=�͹���
���أ�  ��
*/
void THM3070_SleepMode(uint8_t mode)
{
    if(mode)
    {
        GPIO_SetBits(THM_STANDBY_GPIOx, THM_STANDBY_Pinx);                      //���߽���͹���
    }
    else
    {
        GPIO_ResetBits(THM_STANDBY_GPIOx, THM_STANDBY_Pinx);                    //������������
    }
}
/*
���ܣ�  ���ó�ʱʱ��
������  *330us(0-0x00FFFFFF)
���أ�  ��
*/
void THM3070_SetFWT(uint32_t fwt)
{
    THM3070_WriteREG(THM_REG_FWIHIGH, (fwt >> 16) & 0xFF);                      //���ö�ʱ������ֵ
    THM3070_WriteREG(THM_REG_FWIMID, (fwt >> 8) & 0xFF);
    THM3070_WriteREG(THM_REG_FWILOW, fwt & 0xFF);
}

/*
���ܣ�  �л���TYPEAģʽ
����1�� ��
���أ�  ��
*/
void THM3070_SetTYPEA(void)
{
    THM3070_WriteREG(THM_REG_TXCON, 0x72);                                      //����100%
    THM3070_WriteREG(THM_REG_PSEL, 0x10);                                       //ͨ��Э����ΪTYPEA
    //THM3070_SetFrameFormat(2);                                                 //��B����ͬ,������Բ�Ҫ
}

/*
���ܣ�  �л���TYPEBģʽ
����1�� ��
���أ�  ��
*/
void THM3070_SetTYPEB(void)
{
    THM3070_WriteREG(THM_REG_TXCON, 0x62);                                      //����10%
    THM3070_WriteREG(THM_REG_PSEL, 0x00);                                       //ͨ��Э����ΪTYPEB
    THM3070_SetFrameFormat(2);                                                  //��֪��Ϊɶ,û�еĻ��л���MIFARE���л���B������Ӱ��
}
/*
���ܣ�  ���÷�������
����1�� ��������(��������������,0x00:106kbit/s,0x01:212,0x02:424,0x03:848kbit/s)
���أ�  ��
*/
void THM3070_SetTxBaud(uint8_t baud)
{
    uint8_t temp;

    temp = THM3070_ReadREG(THM_REG_PSEL);                                       //�ȶ�ȡ

    baud &= 0x03;                                                               //ֻȡ��2λ
    baud <<= 2;                                                                 //����2λ

    temp &= 0xF3;                                                               //34λ��0
    temp |= baud;                                                               //34λ����

    THM3070_WriteREG(THM_REG_PSEL, temp);                                       //ֻ�ı�34λ
}
/*
���ܣ�  ���ý�������
����1�� ��������(��������������,0x00:106kbit/s,0x01:212,0x02:424,0x03:848kbit/s)
���أ�  ��
*/
void THM3070_SetRxBaud(uint8_t baud)
{
    uint8_t temp;

    temp = THM3070_ReadREG(THM_REG_PSEL);                                       //�ȶ�ȡ

    baud &= 0x03;                                                               //ֻȡ��2λ

    temp &= 0xFC;                                                               //12λ��0
    temp |= baud;                                                               //12λ����

    THM3070_WriteREG(THM_REG_PSEL, temp);                                       //ֻ�ı�12λ
}



/*
���ܣ�  �л���MIFAREģʽ
����1�� ��
���أ�  ��
*/
void THM3070_SetMIFARE(void)
{
    THM3070_WriteREG(THM_REG_TXCON, 0x72);                                      //����100%
    THM3070_WriteREG(THM_REG_PSEL, 0x50);                                       //ͨ��Э����ΪMIFARE
    THM3070_SetFrameFormat(0);                                                  //��֡
}

/*
���ܣ�  ����֡��ʽ
������  ֡��ʽ��0Ϊ��֡,1����ͻ֡����CRC,2Ϊ��׼֡
���أ�  ��
*/
void THM3070_SetFrameFormat(uint8_t format)
{
    if(format == 0)
    {
        THM3070_WriteREG(THM_REG_FMCTRL, 0xC0);
        THM3070_WriteREG(THM_REG_STATCTRL, 0x00);
        THM3070_WriteREG(THM_REG_FMCTRL, 0x40);
        THM3070_WriteREG(THM_REG_CRCSEL, 0x01);                                 //CRC�ر�
    }
    else if(format == 1)
    {
        THM3070_WriteREG(THM_REG_FMCTRL, 0x46);
        THM3070_WriteREG(THM_REG_CRCSEL, 0x01);                                 //CRC�ر�
    }
    else
    {
        THM3070_WriteREG(THM_REG_FMCTRL, 0x42);
        THM3070_WriteREG(THM_REG_CRCSEL, 0xC1);                                 //����CRCʹ��,����CRC�ж�ʹ��,���ճ�ʱ��ʱʹ��
    }
}

/*
���ܣ�  ����֡MIFARE
����1�� ������ݵ�����ָ��
����2�� Ҫ���͵����ݳ���
���أ�  ��
*/
void THM3070_SendFrame_M(uint8_t *p_buff, uint16_t len_buff)
{
    uint8_t temp[1];

    THM3070_WriteREG(THM_REG_SCON, 0x05);                                       //�����ݻ�����
    THM3070_WriteREG(THM_REG_SCON, 0x01);                                       //����״̬

    temp[0] = THM_REG_DATA | 0x80;                                              //д�Ĵ�����λ��1

    THM_IO4_SS_N_L;                                                             //Ƭѡ����ʹ��

    THM3070SPI_SendBuff(temp, 1);                                               //���͵�ַ
    THM3070SPI_SendBuff(p_buff, len_buff);                                      //��������

    THM_IO4_SS_N_H;                                                             //Ƭѡ����

    THM3070_WriteREG(0x1C, 0x01);                                               //��������

}

/*
���ܣ�  ����֡MIFARE
����1�� ������ݵ�����ָ��
����2�� Ҫ���յ����ݳ���
���أ�  ִ�н��
*/
uint8_t THM3070_RecvFrame_M(uint8_t *p_buff, uint16_t *len_buff)
{
    uint8_t RStatus = 0;

    THM3070_TimeCount = 0;
    while(1)                                                                    //�ȴ�������ϻ����
    {
        RStatus = THM3070_ReadREG(0x14);
        if(RStatus & 0xFF)
        {
            break;
        }
        THM3070_TimeCount++;
        if(THM3070_TimeCount > THM3070_TimeCountMax)                            //���볬ʱ�ж�,��������
        {
            THM3070_TimeCount = 0;
            RStatus = THM_RSTST_TMROVER;
            break;
        }
    }
    if(RStatus & 0xEF)                                                          //�жϴ�������
    {
        THM3070_WriteREG(0x14, 0x00);
        return RStatus & 0xEF;
    }

    *len_buff = THM3070_ReadREG(THM_REG_RSCH);
    *len_buff = *len_buff << 8;
    *len_buff |= THM3070_ReadREG(THM_REG_RSCL);                                 //��ȡ���ݳ���
    if(*len_buff > 256)
    {
        return THM_RSTST_DATOVER;
    }

    if(*len_buff > 0)
    {
        uint8_t temp[1];

        THM_IO4_SS_N_L;                                                         //Ƭѡ����ʹ��

        temp[0] = THM_REG_DATA;                                                 //д�Ĵ�����λ��1

        THM3070SPI_SendBuff(temp, 1);                                           //���͵�ַ
        THM3070SPI_RecvBuff(p_buff, *len_buff);                                 //��������

        THM_IO4_SS_N_H;                                                         //Ƭѡ����
    }
    if(RStatus == 0x10)
    {
        return THM_RSTST_FEND;
    }
    else
    {
        return THM_RSTST_OTHER;
    }
}


/*
���ܣ�  �л���ISO15693ģʽ
����1�� ��
���أ�  ��
*/
void THM3070_SetTYPEV(void)
{
    THM3070_WriteREG(THM_REG_PSEL, 0x20);                                       //ͨ��Э����ΪISO15693
    THM3070_WriteREG(THM_REG_SMOD, 0x01);                                       //����1/4,�������100%
}

/*
���ܣ�  ����֡15693
����1�� ������ݵ�����ָ��
����2�� Ҫ���͵����ݳ���
���أ�  ��
*/
void THM3070_SendFrame_V(uint8_t *p_buff, uint16_t len_buff)
{
    uint8_t temp[1];

    THM3070_WriteREG(THM_REG_SCON, 0x05);                                       //�����ݻ�����
    THM3070_WriteREG(THM_REG_SCON, 0x01);                                       //����״̬
    THM3070_WriteREG(THM_REG_EMVERR, 0xFF);                                     //

    temp[0] = THM_REG_DATA | 0x80;                                              //д�Ĵ�����λ��1

    THM_IO4_SS_N_L;                                                             //Ƭѡ����ʹ��

    THM3070SPI_SendBuff(temp, 1);                                               //���͵�ַ
    THM3070SPI_SendBuff(p_buff, len_buff);                                      //��������

    THM_IO4_SS_N_H;                                                             //Ƭѡ����

    THM3070_WriteREG(THM_REG_SCON, 0x03);                                       //��������
    //THM3070_TimeCount=0;
    //while(!THM3070_ReadREG(THM_REG_TXFIN))                                    //�ȴ��������,15693ģʽ�´˼Ĵ�����������
    //{
        //THM3070_TimeCount++;
        //if(THM3070_TimeCount>THM3070_TimeCountMax)                            //���볬ʱ�ж�,��������
        //{
            //THM3070_TimeCount=0;
            //break;
        //}
    //}
}


/*
���ܣ�  ����֡15693
����1�� ������ݵ�����ָ��
����2�� Ҫ���յ����ݳ���
���أ�  ִ�н��
*/
uint8_t THM3070_RecvFrame_V(uint8_t *p_buff, uint16_t *len_buff)
{
    uint8_t EMVError, RStatus = 0;

    THM3070_TimeCount = 0;
    while(1)                                                                    //�ȴ�������ϻ����
    {
        EMVError = THM3070_ReadREG(THM_REG_EMVERR);
        RStatus = THM3070_ReadREG(THM_REG_RSTAT);
        if(RStatus)
        {
            if(EMVError & 0x02)
            {
                return THM_ERROR_EMVERR;
            }
            break;
        }
        THM3070_TimeCount++;
        if(THM3070_TimeCount > THM3070_TimeCountMax)                            //���볬ʱ�ж�,��������
        {
            THM3070_TimeCount = 0;
            RStatus = THM_RSTST_TMROVER;
            break;
        }
    }
    if(RStatus & 0x40)                                                          //�жϴ�������
    {
        RStatus = THM_RSTST_CERR;
        //delay_ms(6);
    }
    else if(RStatus & 0x20)
    {
        RStatus = THM_RSTST_PERR;
    }
    else if(RStatus & 0x10)
    {
        RStatus = THM_RSTST_FERR;
    }
    else if(RStatus & 0x08)
    {
        RStatus = THM_RSTST_DATOVER;
    }
    else if(RStatus & 0x04)
    {
        RStatus = THM_RSTST_TMROVER;
    }
    else if(RStatus & 0x02)
    {
        RStatus = THM_RSTST_CRCERR;
    }
    else
    {
        RStatus = THM_RSTST_FEND;                                               //�޴���
    }

    *len_buff = THM3070_ReadREG(THM_REG_RSCH);
    *len_buff = *len_buff << 8;
    *len_buff |= THM3070_ReadREG(THM_REG_RSCL);                                 //��ȡ���ݳ���
    if(*len_buff > 256)
    {
        return THM_RSTST_DATOVER;
    }

    if(*len_buff > 0)
    {
        uint8_t temp[1];

        THM_IO4_SS_N_L;                                                         //Ƭѡ����ʹ��

        temp[0] = THM_REG_DATA;                                                 //д�Ĵ�����λ��1

        THM3070SPI_SendBuff(temp, 1);                                           //���͵�ַ
        THM3070SPI_RecvBuff(p_buff, *len_buff);                                 //��������

        THM_IO4_SS_N_H;                                                         //Ƭѡ����
    }
    THM3070_WriteREG(THM_REG_RSTAT, 0x00);                                      //���״̬

    return RStatus;
}



