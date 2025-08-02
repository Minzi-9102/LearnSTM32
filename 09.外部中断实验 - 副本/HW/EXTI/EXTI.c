/*********************************************************************************************************
* 模块名称：EXTI.c
* 摘    要：EXTI模块
* 当前版本：1.0.0
* 作    者：SZLY(COPYRIGHT 2018 - 2020 SZLY. All rights reserved.)
* 完成日期：2020年01月01日
* 内    容：
* 注    意：                                                                  
**********************************************************************************************************
* 取代版本：
* 作    者：
* 完成日期：
* 修改内容：
* 修改文件：
*********************************************************************************************************/

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "EXTI.h"
#include "stm32f10x_conf.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
 static i8 count = 0;
/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void ConfigEXTIGPIO(void);				//配置EXTI的GPIO
static void ConfigEXTI(void);						//配置EXTI
/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：ConfigEXTIGPIO
* 函数功能：
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static void ConfigEXTIGPIO(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//上拉输入模式
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA,&GPIO_InitStructure);	
}
/*********************************************************************************************************
* 函数名称：ConfigEXTI
* 函数功能：
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static void ConfigEXTI(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	//PC1
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource1);//选择引脚作为中断线
	EXTI_InitStructure.EXTI_Line = EXTI_Line1;//选择中断线
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//开放中断请求
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;//设置为上升沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;//使能中断线
	EXTI_Init(&EXTI_InitStructure);//根据参数初始化EXTI
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;//中断通道号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;//子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能中断
	NVIC_Init(&NVIC_InitStructure);//根据参数初始化NVIC
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
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitEXTI
* 函数功能：
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
void InitEXTI(void)
{
	ConfigEXTIGPIO();
	ConfigEXTI();
}

