/*********************************************************************************************************
* ģ�����ƣ�EXTI.c
* ժ    Ҫ��EXTIģ��
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
#include "EXTI.h"
#include "stm32f10x_conf.h"

/*********************************************************************************************************
*                                              �궨��
*********************************************************************************************************/

/*********************************************************************************************************
*                                              ö�ٽṹ�嶨��
*********************************************************************************************************/

/*********************************************************************************************************
*                                              �ڲ�����
*********************************************************************************************************/
 static i8 count = 0;
/*********************************************************************************************************
*                                              �ڲ���������
*********************************************************************************************************/
static void ConfigEXTIGPIO(void);				//����EXTI��GPIO
static void ConfigEXTI(void);						//����EXTI
/*********************************************************************************************************
*                                              �ڲ�����ʵ��
*********************************************************************************************************/
/*********************************************************************************************************
* �������ƣ�ConfigEXTIGPIO
* �������ܣ�
* ���������
* ���������
* �� �� ֵ��
* �������ڣ�2018��01��01��
* ע    �⣺
*********************************************************************************************************/
static void ConfigEXTIGPIO(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//��������ģʽ
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA,&GPIO_InitStructure);	
}
/*********************************************************************************************************
* �������ƣ�ConfigEXTI
* �������ܣ�
* ���������
* ���������
* �� �� ֵ��
* �������ڣ�2018��01��01��
* ע    �⣺
*********************************************************************************************************/
static void ConfigEXTI(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	//PC1
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource1);//ѡ��������Ϊ�ж���
	EXTI_InitStructure.EXTI_Line = EXTI_Line1;//ѡ���ж���
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�����ж�����
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;//����Ϊ�����ش���
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;//ʹ���ж���
	EXTI_Init(&EXTI_InitStructure);//���ݲ�����ʼ��EXTI
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;//�ж�ͨ����
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;//�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//ʹ���ж�
	NVIC_Init(&NVIC_InitStructure);//���ݲ�����ʼ��NVIC
	//PC2
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource2);
	EXTI_InitStructure.EXTI_Line = EXTI_Line2;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	//PA0
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource0);
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void EXTI0_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line0) != RESET)
	{
	if(--count == -1)
		{count = 3;}
switch(count)
		{
			case 0:
			//00
			GPIO_WriteBit(GPIOC,GPIO_Pin_4,Bit_RESET);
			GPIO_WriteBit(GPIOC,GPIO_Pin_5,Bit_RESET);
			break;
			case 1:
			//01
			GPIO_WriteBit(GPIOC,GPIO_Pin_4,Bit_RESET);
			GPIO_WriteBit(GPIOC,GPIO_Pin_5,Bit_SET);
			break;
			case 2:
			//10
			GPIO_WriteBit(GPIOC,GPIO_Pin_4,Bit_SET);
			GPIO_WriteBit(GPIOC,GPIO_Pin_5,Bit_RESET);
			break;
			case 3:
			//10
			GPIO_WriteBit(GPIOC,GPIO_Pin_4,Bit_SET);
			GPIO_WriteBit(GPIOC,GPIO_Pin_5,Bit_SET);
			break;
			default:break;
		}
		EXTI_ClearITPendingBit(EXTI_Line0);		
	}
}

void EXTI1_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line1) != RESET)
	{
		if(++count == 4)
		{count = 0;}
		
		switch(count)
		{
			case 0:
			//00
			GPIO_WriteBit(GPIOC,GPIO_Pin_4,Bit_RESET);
			GPIO_WriteBit(GPIOC,GPIO_Pin_5,Bit_RESET);
			break;
			case 1:
			//01
			GPIO_WriteBit(GPIOC,GPIO_Pin_4,Bit_RESET);
			GPIO_WriteBit(GPIOC,GPIO_Pin_5,Bit_SET);
			break;
			case 2:
			//10
			GPIO_WriteBit(GPIOC,GPIO_Pin_4,Bit_SET);
			GPIO_WriteBit(GPIOC,GPIO_Pin_5,Bit_RESET);
			break;
			case 3:
			//10
			GPIO_WriteBit(GPIOC,GPIO_Pin_4,Bit_SET);
			GPIO_WriteBit(GPIOC,GPIO_Pin_5,Bit_SET);
			break;
			default:break;
		}
		
		
		EXTI_ClearITPendingBit(EXTI_Line1);		
	}
}

void EXTI2_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line2) != RESET)
	{
		count = 0;
		GPIO_WriteBit(GPIOC,GPIO_Pin_4,Bit_RESET);
		GPIO_WriteBit(GPIOC,GPIO_Pin_5,Bit_RESET);
		EXTI_ClearITPendingBit(EXTI_Line2);		
	}
}

/*********************************************************************************************************
*                                              API����ʵ��
*********************************************************************************************************/
/*********************************************************************************************************
* �������ƣ�InitEXTI
* �������ܣ�
* ���������
* ���������
* �� �� ֵ��
* �������ڣ�2018��01��01��
* ע    �⣺
*********************************************************************************************************/
void InitEXTI(void)
{
	ConfigEXTIGPIO();
	ConfigEXTI();
}

