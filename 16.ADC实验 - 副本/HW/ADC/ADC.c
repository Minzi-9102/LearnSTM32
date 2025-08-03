/*********************************************************************************************************
* ģ�����ƣ�ADC.c
* ժ    Ҫ��ADCģ��
* ��ǰ�汾��1.0.0
* ��    �ߣ�SZLY(COPYRIGHT 2018 - 2020 SZLY. All rights reserved.)
* ������ڣ�2020��01��01��
* ��    �ݣ�
* ע    �⣺                                                                  
**********************************************************************************************************
* ȡ���汾��
* ��    �ߣ�
* ������ڣ�
* �޸����ݣ�
* �޸��ļ���
*********************************************************************************************************/
/*********************************************************************************************************
*                                              ����ͷ�ļ�
*********************************************************************************************************/
#include "ADC.h"
#include "stm32f10x_conf.h"
#include "U16Queue.h"
/*********************************************************************************************************
*                                              �궨��
*********************************************************************************************************/

/*********************************************************************************************************
*                                              ö�ٽṹ�嶨��
*********************************************************************************************************/

/*********************************************************************************************************
*                                              �ڲ�����
*********************************************************************************************************/
 static u16 s_arrADC1Data;//���ADC1ת���������
 static StructU16CirQue s_structADCCirQue;//ADCѭ������
 static u16 s_arrADCBuf[ADC1_BUF_SIZE];//ADCѭ�����еĻ�����
/*********************************************************************************************************
*                                              �ڲ���������
*********************************************************************************************************/
static void ConfigADC1(void);//����ADC1
static void ConfigDMA1Ch1(void);//����DMA1ͨ��1
static void ConfigTimer3(u16 arr,u16 psc);//����TIM3
/*********************************************************************************************************
*                                              �ڲ�����ʵ��
*********************************************************************************************************/
/*********************************************************************************************************
* �������ƣ�ConfigADC1
* �������ܣ�����ADC1
* ���������
* ���������
* �� �� ֵ��
* �������ڣ�2018��01��01��
* ע    �⣺
*********************************************************************************************************/
static void ConfigADC1(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;//GPIO��ʼ���ṹ��
	ADC_InitTypeDef ADC_InitStructure;//ADC��ʼ���ṹ��

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);//����ADCʱ��,ADCCLK = PCLK2/6 = 72MHz/6 = 12MHz
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);//ʹ��ADC1ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//ʹ��GPIOAʱ��
	
	//����PA3Ϊģ������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//����GPIOA3����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;//����GPIOA3����Ϊģ������
	GPIO_Init(GPIOA,&GPIO_InitStructure);//��ʼ��GPIOA3����
	
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;//����ADC����ģʽΪ����ģʽ
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;//����ADCɨ��ת��ģʽΪ����
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;//����ADC����ת��ģʽΪ�ر�
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;//����ADC�ⲿ����ת��ΪTIM3����
	
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//����ADC���ݶ��뷽ʽΪ�Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 1;//����ADCת��ͨ����Ϊ1
	
	ADC_Init(ADC1,&ADC_InitStructure);//��ʼ��ADC1
	
	ADC_RegularChannelConfig(ADC1,ADC_Channel_3,1,ADC_SampleTime_239Cycles5);//����ADC1ͨ��3��
																			//���ò���ʱ��Ϊ239.5��ʱ������
	
	ADC_DMACmd(ADC1,ENABLE);//ʹ��ADC1��DMA����
	ADC_ExternalTrigConvCmd(ADC1,ENABLE);//ʹ��ADC1�ⲿ����ת��
	ADC_Cmd(ADC1,ENABLE);//ʹ��ADC1
	ADC_ResetCalibration(ADC1);//��λADC1
	while(ADC_GetResetCalibrationStatus(ADC1));//�ȴ���λ���
	ADC_StartCalibration(ADC1);//��ʼADC1У׼
	while(ADC_GetCalibrationStatus(ADC1));//�ȴ�У׼���
}
/*********************************************************************************************************
* �������ƣ�ConfigDMA1Ch1
* �������ܣ�
* ���������
* ���������
* �� �� ֵ��
* �������ڣ�2018��01��01��
* ע    �⣺
*********************************************************************************************************/
static void ConfigDMA1Ch1(void)
{
	DMA_InitTypeDef DMA_InitStructure;//DMA��ʼ���ṹ��?
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);//ʹ��DMA1ʱ��
	
	DMA_DeInit(DMA1_Channel1);//��λDMA1ͨ��1
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(ADC1->DR);//����DMA�����ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&s_arrADC1Data;//����DMA�ڴ��ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//����DMA���䷽��Ϊ���赽�ڴ�
	DMA_InitStructure.DMA_BufferSize = 1;//����DMA��������СΪ1�����ݵ�λ
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//����DMA�����ַ������
	
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//����DMA�ڴ��ַ����
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//����DMA�������ݴ�СΪ����
	
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//����DMA�ڴ����ݴ�СΪ����
	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//����DMA����ģʽΪѭ��ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;//����DMA���ȼ�Ϊ�е�
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//����DMA�ڴ浽�ڴ洫��Ϊ����
	
	DMA_Init(DMA1_Channel1,&DMA_InitStructure);//��ʼ��DMA1ͨ��1
	
	DMA_Cmd(DMA1_Channel1,ENABLE);//ʹ��DMA1ͨ��1
}

/*********************************************************************************************************
* �������ƣ�ConfigTimer3
* �������ܣ�
* ���������
* ���������
* �� �� ֵ��
* �������ڣ�2018��01��01��
* ע    �⣺
*********************************************************************************************************/
static void ConfigTimer3(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;//��ʱ��������ʼ���ṹ��
	NVIC_InitTypeDef NVIC_InitStructure;//�ж�������ʼ���ṹ��
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);//ʹ��TIM3ʱ��
	
	TIM_TimeBaseStructure.TIM_Period = arr;//���ö�ʱ������
	TIM_TimeBaseStructure.TIM_Prescaler = psc;//���ö�ʱ��Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//���ö�ʱ��ʱ�ӷ�ƵΪ1
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//���ö�ʱ������ģʽΪ���ϼ���
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);//��ʼ��TIM3
	
	TIM_SelectOutputTrigger(TIM3,TIM_TRGOSource_Update);//ѡ��TIM3���������ԴΪ�����¼�
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);//ʹ��TIM3�����ж�
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;//��ʱ��3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//������ռ���ȼ�Ϊ1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;//������Ӧ���ȼ�Ϊ1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//ʹ�ܶ�ʱ��3�ж�
	NVIC_Init(&NVIC_InitStructure);//��ʼ���ж�����
	
	TIM_Cmd(TIM3,ENABLE);//ʹ�ܶ�ʱ��3
}

/*********************************************************************************************************
* �������ƣ�TIM3_IRQHandler
* �������ܣ�
* ���������
* ���������
* �� �� ֵ��
* �������ڣ�2018��01��01��
* ע    �⣺
*********************************************************************************************************/
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update) != RESET)//���TIM3�����жϱ�־λ�Ƿ���λ
	{
		TIM_ClearITPendingBit(TIM3,TIM_FLAG_Update);//���TIM3�����жϱ�־λ
	}
	WriteADCBuf(s_arrADC1Data);//��ADC1ת���������д��ѭ������
}
/*********************************************************************************************************
*                                              API����ʵ��
*********************************************************************************************************/
/*********************************************************************************************************
* �������ƣ�InitADC
* �������ܣ�
* ���������
* ���������
* �� �� ֵ��
* �������ڣ�2018��01��01��
* ע    �⣺
*********************************************************************************************************/
void InitADC(void)
{
	ConfigTimer3(799,719);//����TIM3��ARR=799,PSC=719
	//TIM3����ADC1ת��������Ƶ��Ϊ100Hz
	ConfigADC1();//����ADC1
	ConfigDMA1Ch1();//����DMA1ͨ��1
	
	InitU16Queue(&s_structADCCirQue,s_arrADCBuf,ADC1_BUF_SIZE);//��ʼ��ADCѭ������
}

/*********************************************************************************************************
* �������ƣ�WriteADCBuf
* �������ܣ�
* ���������
* ���������
* �� �� ֵ��
* �������ڣ�2018��01��01��
* ע    �⣺
*********************************************************************************************************/
u8 WriteADCBuf(u16 d)
{
	u8 ok = 0;//����ֵ��0��ʾʧ�ܣ�1��ʾ�ɹ�
	ok = EnU16Queue(&s_structADCCirQue,&d,1);//��ADC1ת���������д��ѭ������
	return ok;//���ز������
}

/*********************************************************************************************************
* �������ƣ�ReadADCBuf
* �������ܣ�
* ���������
* ���������
* �� �� ֵ��
* �������ڣ�2018��01��01��
* ע    �⣺
*********************************************************************************************************/
u8 ReadADCBuf(u16* p)
{
	u8 ok = 0;//����ֵ��0��ʾʧ�ܣ�1��ʾ�ɹ�
	ok = DeU16Queue(&s_structADCCirQue,p,1);//��ADCѭ�������ж�ȡ1������
	return ok;	//���ز������
}

