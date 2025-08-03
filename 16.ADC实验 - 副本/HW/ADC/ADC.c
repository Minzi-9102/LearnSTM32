/*********************************************************************************************************
* 模块名称：ADC.c
* 摘    要：ADC模块
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
#include "ADC.h"
#include "stm32f10x_conf.h"
#include "U16Queue.h"
/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
 static u16 s_arrADC1Data;//存放ADC1转换结果数据
 static StructU16CirQue s_structADCCirQue;//ADC循环队列
 static u16 s_arrADCBuf[ADC1_BUF_SIZE];//ADC循环队列的缓冲区
/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void ConfigADC1(void);//配置ADC1
static void ConfigDMA1Ch1(void);//配置DMA1通道1
static void ConfigTimer3(u16 arr,u16 psc);//配置TIM3
/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：ConfigADC1
* 函数功能：配置ADC1
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static void ConfigADC1(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;//GPIO初始化结构体
	ADC_InitTypeDef ADC_InitStructure;//ADC初始化结构体

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);//配置ADC时钟,ADCCLK = PCLK2/6 = 72MHz/6 = 12MHz
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);//使能ADC1时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能GPIOA时钟
	
	//配置PA3为模拟输入
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//设置GPIOA3引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;//设置GPIOA3引脚为模拟输入
	GPIO_Init(GPIOA,&GPIO_InitStructure);//初始化GPIOA3引脚
	
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;//设置ADC工作模式为独立模式
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;//设置ADC扫描转换模式为开启
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;//设置ADC连续转换模式为关闭
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;//设置ADC外部触发转换为TIM3触发
	
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//设置ADC数据对齐方式为右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;//设置ADC转换通道数为1
	
	ADC_Init(ADC1,&ADC_InitStructure);//初始化ADC1
	
	ADC_RegularChannelConfig(ADC1,ADC_Channel_3,1,ADC_SampleTime_239Cycles5);//配置ADC1通道3，
																			//设置采样时间为239.5个时钟周期
	
	ADC_DMACmd(ADC1,ENABLE);//使能ADC1的DMA功能
	ADC_ExternalTrigConvCmd(ADC1,ENABLE);//使能ADC1外部触发转换
	ADC_Cmd(ADC1,ENABLE);//使能ADC1
	ADC_ResetCalibration(ADC1);//复位ADC1
	while(ADC_GetResetCalibrationStatus(ADC1));//等待复位完成
	ADC_StartCalibration(ADC1);//开始ADC1校准
	while(ADC_GetCalibrationStatus(ADC1));//等待校准完成
}
/*********************************************************************************************************
* 函数名称：ConfigDMA1Ch1
* 函数功能：
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static void ConfigDMA1Ch1(void)
{
	DMA_InitTypeDef DMA_InitStructure;//DMA初始化结构体?
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);//使能DMA1时钟
	
	DMA_DeInit(DMA1_Channel1);//复位DMA1通道1
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(ADC1->DR);//设置DMA外设地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&s_arrADC1Data;//设置DMA内存地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//设置DMA传输方向为外设到内存
	DMA_InitStructure.DMA_BufferSize = 1;//设置DMA缓冲区大小为1个数据单位
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//设置DMA外设地址不递增
	
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//设置DMA内存地址递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//设置DMA外设数据大小为半字
	
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//设置DMA内存数据大小为半字
	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//设置DMA工作模式为循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;//设置DMA优先级为中等
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//设置DMA内存到内存传输为禁用
	
	DMA_Init(DMA1_Channel1,&DMA_InitStructure);//初始化DMA1通道1
	
	DMA_Cmd(DMA1_Channel1,ENABLE);//使能DMA1通道1
}

/*********************************************************************************************************
* 函数名称：ConfigTimer3
* 函数功能：
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static void ConfigTimer3(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;//定时器基础初始化结构体
	NVIC_InitTypeDef NVIC_InitStructure;//中断向量初始化结构体
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);//使能TIM3时钟
	
	TIM_TimeBaseStructure.TIM_Period = arr;//设置定时器周期
	TIM_TimeBaseStructure.TIM_Prescaler = psc;//设置定时器预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//设置定时器时钟分频为1
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//设置定时器计数模式为向上计数
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);//初始化TIM3
	
	TIM_SelectOutputTrigger(TIM3,TIM_TRGOSource_Update);//选择TIM3的输出触发源为更新事件
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);//使能TIM3更新中断
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;//定时器3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//设置抢占优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;//设置响应优先级为1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能定时器3中断
	NVIC_Init(&NVIC_InitStructure);//初始化中断向量
	
	TIM_Cmd(TIM3,ENABLE);//使能定时器3
}

/*********************************************************************************************************
* 函数名称：TIM3_IRQHandler
* 函数功能：
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update) != RESET)//检查TIM3更新中断标志位是否被置位
	{
		TIM_ClearITPendingBit(TIM3,TIM_FLAG_Update);//清除TIM3更新中断标志位
	}
	WriteADCBuf(s_arrADC1Data);//将ADC1转换结果数据写入循环队列
}
/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitADC
* 函数功能：
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
void InitADC(void)
{
	ConfigTimer3(799,719);//配置TIM3，ARR=799,PSC=719
	//TIM3触发ADC1转换，采样频率为100Hz
	ConfigADC1();//配置ADC1
	ConfigDMA1Ch1();//配置DMA1通道1
	
	InitU16Queue(&s_structADCCirQue,s_arrADCBuf,ADC1_BUF_SIZE);//初始化ADC循环队列
}

/*********************************************************************************************************
* 函数名称：WriteADCBuf
* 函数功能：
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
u8 WriteADCBuf(u16 d)
{
	u8 ok = 0;//返回值，0表示失败，1表示成功
	ok = EnU16Queue(&s_structADCCirQue,&d,1);//将ADC1转换结果数据写入循环队列
	return ok;//返回操作结果
}

/*********************************************************************************************************
* 函数名称：ReadADCBuf
* 函数功能：
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
u8 ReadADCBuf(u16* p)
{
	u8 ok = 0;//返回值，0表示失败，1表示成功
	ok = DeU16Queue(&s_structADCCirQue,p,1);//从ADC循环队列中读取1个数据
	return ok;	//返回操作结果
}

